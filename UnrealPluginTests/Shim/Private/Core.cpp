// Copyright (C) Microsoft Corporation. All rights reserved.

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cwchar>
#include <cwctype>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
#include <thread>

#if defined(_WIN32)
#include <process.h>
#else
#include <unistd.h>
#endif

#include "ShimInternal.h"

// ---------------------------------------------------------------------------------------------------------------------
// Diagnostics and string conversion

void UEShim::CheckFailed(const char* Expression, const char* File, int Line)
{
	std::fprintf(stderr, "UE shim assertion failed: %s [%s:%d]\n", Expression, File, Line);
	std::fflush(stdout);
	std::fflush(stderr);
	std::abort();
}

namespace
{
	void AppendUtf8(std::string& Out, uint32 CodePoint)
	{
		if (CodePoint < 0x80)
		{
			Out += static_cast<char>(CodePoint);
		}
		else if (CodePoint < 0x800)
		{
			Out += static_cast<char>(0xC0 | (CodePoint >> 6));
			Out += static_cast<char>(0x80 | (CodePoint & 0x3F));
		}
		else if (CodePoint < 0x10000)
		{
			Out += static_cast<char>(0xE0 | (CodePoint >> 12));
			Out += static_cast<char>(0x80 | ((CodePoint >> 6) & 0x3F));
			Out += static_cast<char>(0x80 | (CodePoint & 0x3F));
		}
		else
		{
			Out += static_cast<char>(0xF0 | (CodePoint >> 18));
			Out += static_cast<char>(0x80 | ((CodePoint >> 12) & 0x3F));
			Out += static_cast<char>(0x80 | ((CodePoint >> 6) & 0x3F));
			Out += static_cast<char>(0x80 | (CodePoint & 0x3F));
		}
	}

	void AppendWide(std::wstring& Out, uint32 CodePoint)
	{
		if constexpr (sizeof(wchar_t) == 2)
		{
			if (CodePoint >= 0x10000)
			{
				CodePoint -= 0x10000;
				Out += static_cast<wchar_t>(0xD800 + (CodePoint >> 10));
				Out += static_cast<wchar_t>(0xDC00 + (CodePoint & 0x3FF));
				return;
			}
		}
		Out += static_cast<wchar_t>(CodePoint);
	}
}

std::string UEShim::ToUtf8(const std::wstring& Wide)
{
	std::string Out;
	for (std::size_t Index = 0; Index < Wide.size(); ++Index)
	{
		uint32 CodePoint = static_cast<uint32>(Wide[Index]);
		if (sizeof(wchar_t) == 2 && CodePoint >= 0xD800 && CodePoint <= 0xDBFF && Index + 1 < Wide.size())
		{
			const uint32 Low = static_cast<uint32>(Wide[Index + 1]);
			if (Low >= 0xDC00 && Low <= 0xDFFF)
			{
				CodePoint = 0x10000 + ((CodePoint - 0xD800) << 10) + (Low - 0xDC00);
				++Index;
			}
		}
		AppendUtf8(Out, CodePoint);
	}
	return Out;
}

std::wstring UEShim::FromUtf8(const char* Utf8, std::size_t Length)
{
	std::wstring Out;
	const unsigned char* Bytes = reinterpret_cast<const unsigned char*>(Utf8);
	std::size_t Index = 0;
	while (Index < Length)
	{
		const unsigned char Lead = Bytes[Index];
		uint32 CodePoint = 0xFFFD;
		std::size_t Count = 1;
		if (Lead < 0x80)
		{
			CodePoint = Lead;
		}
		else if ((Lead >> 5) == 0x6 && Index + 1 < Length)
		{
			CodePoint = ((Lead & 0x1F) << 6) | (Bytes[Index + 1] & 0x3F);
			Count = 2;
		}
		else if ((Lead >> 4) == 0xE && Index + 2 < Length)
		{
			CodePoint = ((Lead & 0x0F) << 12) | ((Bytes[Index + 1] & 0x3F) << 6) | (Bytes[Index + 2] & 0x3F);
			Count = 3;
		}
		else if ((Lead >> 3) == 0x1E && Index + 3 < Length)
		{
			CodePoint = ((Lead & 0x07) << 18) | ((Bytes[Index + 1] & 0x3F) << 12) | ((Bytes[Index + 2] & 0x3F) << 6) | (Bytes[Index + 3] & 0x3F);
			Count = 4;
		}
		AppendWide(Out, CodePoint);
		Index += Count;
	}
	return Out;
}

// ---------------------------------------------------------------------------------------------------------------------
// FString

namespace
{
	wchar_t FoldCase(wchar_t Char)
	{
		return static_cast<wchar_t>(std::towlower(static_cast<std::wint_t>(Char)));
	}

	bool CharsEqual(wchar_t A, wchar_t B, ESearchCase::Type SearchCase)
	{
		return SearchCase == ESearchCase::CaseSensitive ? A == B : FoldCase(A) == FoldCase(B);
	}
}

bool FString::Equals(const FString& Other, ESearchCase::Type SearchCase) const
{
	if (Data.size() != Other.Data.size())
	{
		return false;
	}
	for (std::size_t Index = 0; Index < Data.size(); ++Index)
	{
		if (!CharsEqual(Data[Index], Other.Data[Index], SearchCase))
		{
			return false;
		}
	}
	return true;
}

int32 FString::Compare(const FString& Other, ESearchCase::Type SearchCase) const
{
	const std::size_t Common = std::min(Data.size(), Other.Data.size());
	for (std::size_t Index = 0; Index < Common; ++Index)
	{
		const wchar_t A = SearchCase == ESearchCase::CaseSensitive ? Data[Index] : FoldCase(Data[Index]);
		const wchar_t B = SearchCase == ESearchCase::CaseSensitive ? Other.Data[Index] : FoldCase(Other.Data[Index]);
		if (A != B)
		{
			return A < B ? -1 : 1;
		}
	}
	if (Data.size() == Other.Data.size())
	{
		return 0;
	}
	return Data.size() < Other.Data.size() ? -1 : 1;
}

int32 FString::Find(const FString& SubStr, ESearchCase::Type SearchCase) const
{
	if (SubStr.Data.size() > Data.size())
	{
		return -1;
	}
	for (std::size_t Start = 0; Start + SubStr.Data.size() <= Data.size(); ++Start)
	{
		bool bMatch = true;
		for (std::size_t Index = 0; Index < SubStr.Data.size(); ++Index)
		{
			if (!CharsEqual(Data[Start + Index], SubStr.Data[Index], SearchCase))
			{
				bMatch = false;
				break;
			}
		}
		if (bMatch)
		{
			return static_cast<int32>(Start);
		}
	}
	return -1;
}

bool FString::StartsWith(const FString& Prefix, ESearchCase::Type SearchCase) const
{
	return Prefix.Data.size() <= Data.size() && FString(Data.substr(0, Prefix.Data.size())).Equals(Prefix, SearchCase);
}

bool FString::EndsWith(const FString& Suffix, ESearchCase::Type SearchCase) const
{
	return Suffix.Data.size() <= Data.size() && FString(Data.substr(Data.size() - Suffix.Data.size())).Equals(Suffix, SearchCase);
}

bool FString::IsNumeric() const
{
	if (Data.empty())
	{
		return false;
	}
	std::size_t Index = (Data[0] == L'-' || Data[0] == L'+') ? 1 : 0;
	bool bHasDigit = false;
	bool bHasDot = false;
	for (; Index < Data.size(); ++Index)
	{
		if (Data[Index] == L'.' && !bHasDot)
		{
			bHasDot = true;
		}
		else if (std::iswdigit(static_cast<std::wint_t>(Data[Index])))
		{
			bHasDigit = true;
		}
		else
		{
			return false;
		}
	}
	return bHasDigit;
}

FString FString::ToLower() const
{
	std::wstring Lower = Data;
	std::transform(Lower.begin(), Lower.end(), Lower.begin(), FoldCase);
	return FString(std::move(Lower));
}

FString FString::SanitizeFloat(double Value, int32 MinFractionalDigits)
{
	wchar_t Buffer[64];
	std::swprintf(Buffer, 64, L"%f", Value);
	std::wstring Result = Buffer;
	const std::size_t Dot = Result.find(L'.');
	if (Dot != std::wstring::npos)
	{
		std::size_t LastKept = Result.size() - 1;
		while (LastKept > Dot + static_cast<std::size_t>(MinFractionalDigits) && Result[LastKept] == L'0')
		{
			--LastKept;
		}
		Result.erase(LastKept + 1);
		if (!Result.empty() && Result.back() == L'.')
		{
			Result.pop_back();
		}
	}
	return FString(std::move(Result));
}

// Unreal's printf treats %s and %c as TCHAR arguments; the C library's wide printf needs %ls / %lc for that.
namespace
{
	std::wstring TranslateFormat(const TCHAR* Format)
	{
		std::wstring Out;
		const std::wstring In = Format;
		std::size_t Index = 0;
		while (Index < In.size())
		{
			if (In[Index] != L'%')
			{
				Out += In[Index++];
				continue;
			}
			Out += In[Index++];
			if (Index < In.size() && In[Index] == L'%')
			{
				Out += In[Index++];
				continue;
			}
			while (Index < In.size() && std::wcschr(L"-+ #0", In[Index]) != nullptr)
			{
				Out += In[Index++];
			}
			while (Index < In.size() && (std::iswdigit(static_cast<std::wint_t>(In[Index])) || In[Index] == L'*' || In[Index] == L'.'))
			{
				Out += In[Index++];
			}
			std::wstring Length;
			while (Index < In.size() && std::wcschr(L"hlLjzt", In[Index]) != nullptr)
			{
				Length += In[Index++];
			}
			if (Index >= In.size())
			{
				Out += Length;
				break;
			}
			const wchar_t Conversion = In[Index++];
			if ((Conversion == L's' || Conversion == L'c') && Length.empty())
			{
				Length = L"l";
			}
			Out += Length;
			Out += Conversion;
		}
		return Out;
	}
}

FString UEShim::FormatString(const TCHAR* Format, ...)
{
	const std::wstring Translated = TranslateFormat(Format);
	va_list Args;
	va_start(Args, Format);
	std::vector<wchar_t> Buffer(512);
	FString Result;
	while (true)
	{
		va_list ArgsCopy;
		va_copy(ArgsCopy, Args);
		const int Written = std::vswprintf(Buffer.data(), Buffer.size(), Translated.c_str(), ArgsCopy);
		va_end(ArgsCopy);
		if (Written >= 0 && static_cast<std::size_t>(Written) < Buffer.size())
		{
			Result = FString(std::wstring(Buffer.data(), Written));
			break;
		}
		if (Buffer.size() >= (1u << 22))
		{
			Result = FString(L"<UE shim: invalid format string>");
			break;
		}
		Buffer.resize(Buffer.size() * 4);
	}
	va_end(Args);
	return Result;
}

// ---------------------------------------------------------------------------------------------------------------------
// Logging

FOutputDeviceRedirector* GLog = new FOutputDeviceRedirector();

void FOutputDeviceRedirector::AddOutputDevice(FOutputDevice* Device)
{
	std::lock_guard<std::mutex> Lock(Mutex);
	if (Device != nullptr && std::find(Devices.begin(), Devices.end(), Device) == Devices.end())
	{
		Devices.push_back(Device);
	}
}

void FOutputDeviceRedirector::RemoveOutputDevice(FOutputDevice* Device)
{
	std::lock_guard<std::mutex> Lock(Mutex);
	Devices.erase(std::remove(Devices.begin(), Devices.end(), Device), Devices.end());
}

void FOutputDeviceRedirector::Broadcast(const TCHAR* Message, ELogVerbosity::Type Verbosity, const TCHAR* Category)
{
	std::lock_guard<std::mutex> Lock(Mutex);
	for (FOutputDevice* Device : Devices)
	{
		Device->Serialize(Message, Verbosity, Category);
	}
}

namespace
{
	const char* VerbosityName(ELogVerbosity::Type Verbosity)
	{
		switch (Verbosity)
		{
		case ELogVerbosity::Fatal: return "Fatal";
		case ELogVerbosity::Error: return "Error";
		case ELogVerbosity::Warning: return "Warning";
		case ELogVerbosity::Display: return "Display";
		case ELogVerbosity::Log: return "Log";
		default: return "Verbose";
		}
	}

	std::mutex& OutputMutex()
	{
		static std::mutex* Mutex = new std::mutex();
		return *Mutex;
	}
}

void UEShim::LogMessage(const FLogCategoryBase& Category, ELogVerbosity::Type Verbosity, const FString& Message)
{
	const bool bExpected = CaptureLogForRunningTest(Message, Verbosity, Category.Name);
	{
		std::lock_guard<std::mutex> Lock(OutputMutex());
		std::printf("%s: %s: %s%s\n", ToUtf8(Category.Name).c_str(), VerbosityName(Verbosity), ToUtf8(Message.GetStdString()).c_str(), bExpected ? " (expected)" : "");
		std::fflush(stdout);
	}
	if (GLog != nullptr)
	{
		GLog->Broadcast(*Message, Verbosity, Category.Name);
	}
	if (Verbosity == ELogVerbosity::Fatal)
	{
		std::fprintf(stderr, "Fatal error logged; aborting like Unreal would.\n");
		std::fflush(stderr);
		std::abort();
	}
}

// ---------------------------------------------------------------------------------------------------------------------
// FDateTime

namespace
{
	constexpr int64 UnixEpochTicks = 621355968000000000LL;

	// Days since 1970-01-01 in the proleptic Gregorian calendar (H. Hinnant's days_from_civil).
	int64 DaysFromCivil(int64 Year, unsigned Month, unsigned Day)
	{
		Year -= Month <= 2;
		const int64 Era = (Year >= 0 ? Year : Year - 399) / 400;
		const unsigned YearOfEra = static_cast<unsigned>(Year - Era * 400);
		const unsigned DayOfYear = (153 * (Month > 2 ? Month - 3 : Month + 9) + 2) / 5 + Day - 1;
		const unsigned DayOfEra = YearOfEra * 365 + YearOfEra / 4 - YearOfEra / 100 + DayOfYear;
		return Era * 146097 + static_cast<int64>(DayOfEra) - 719468;
	}

	bool ReadDigits(const std::wstring& Text, std::size_t& Index, std::size_t Count, int32& OutValue)
	{
		OutValue = 0;
		for (std::size_t Digit = 0; Digit < Count; ++Digit, ++Index)
		{
			if (Index >= Text.size() || !std::iswdigit(static_cast<std::wint_t>(Text[Index])))
			{
				return false;
			}
			OutValue = OutValue * 10 + (Text[Index] - L'0');
		}
		return true;
	}
}

FDateTime::FDateTime(int32 Year, int32 Month, int32 Day, int32 Hour, int32 Minute, int32 Second, int32 Millisecond)
	: Ticks(UnixEpochTicks + DaysFromCivil(Year, static_cast<unsigned>(Month), static_cast<unsigned>(Day)) * ETimespan::TicksPerDay
		+ Hour * ETimespan::TicksPerHour + Minute * ETimespan::TicksPerMinute + Second * ETimespan::TicksPerSecond + Millisecond * ETimespan::TicksPerMillisecond)
{
}

FDateTime FDateTime::UtcNow()
{
	const auto SinceEpoch = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch());
	return FDateTime(UnixEpochTicks + SinceEpoch.count() * 10);
}

FDateTime FDateTime::Now()
{
	return UtcNow();
}

int64 FDateTime::ToUnixTimestamp() const
{
	return (Ticks - UnixEpochTicks) / ETimespan::TicksPerSecond;
}

bool FDateTime::ParseIso8601(const TCHAR* DateTimeString, FDateTime& OutDateTime)
{
	const std::wstring Text = DateTimeString ? DateTimeString : L"";
	std::size_t Index = 0;
	int32 Year = 0, Month = 0, Day = 0, Hour = 0, Minute = 0, Second = 0;
	if (!ReadDigits(Text, Index, 4, Year) || Index >= Text.size() || Text[Index++] != L'-'
		|| !ReadDigits(Text, Index, 2, Month) || Index >= Text.size() || Text[Index++] != L'-'
		|| !ReadDigits(Text, Index, 2, Day))
	{
		return false;
	}

	int64 FractionTicks = 0;
	int64 OffsetTicks = 0;
	if (Index < Text.size() && (Text[Index] == L'T' || Text[Index] == L't'))
	{
		++Index;
		if (!ReadDigits(Text, Index, 2, Hour) || Index >= Text.size() || Text[Index++] != L':' || !ReadDigits(Text, Index, 2, Minute))
		{
			return false;
		}
		if (Index < Text.size() && Text[Index] == L':')
		{
			++Index;
			if (!ReadDigits(Text, Index, 2, Second))
			{
				return false;
			}
			if (Index < Text.size() && Text[Index] == L'.')
			{
				++Index;
				int64 Scale = ETimespan::TicksPerSecond / 10;
				while (Index < Text.size() && std::iswdigit(static_cast<std::wint_t>(Text[Index])))
				{
					FractionTicks += (Text[Index++] - L'0') * Scale;
					Scale /= 10;
				}
			}
		}
		if (Index < Text.size() && (Text[Index] == L'Z' || Text[Index] == L'z'))
		{
			++Index;
		}
		else if (Index < Text.size() && (Text[Index] == L'+' || Text[Index] == L'-'))
		{
			const int64 Sign = Text[Index++] == L'+' ? 1 : -1;
			int32 OffsetHours = 0, OffsetMinutes = 0;
			if (!ReadDigits(Text, Index, 2, OffsetHours))
			{
				return false;
			}
			if (Index < Text.size() && Text[Index] == L':')
			{
				++Index;
			}
			if (Index < Text.size() && !ReadDigits(Text, Index, 2, OffsetMinutes))
			{
				return false;
			}
			OffsetTicks = Sign * (OffsetHours * ETimespan::TicksPerHour + OffsetMinutes * ETimespan::TicksPerMinute);
		}
	}

	if (Index != Text.size() || Month < 1 || Month > 12 || Day < 1 || Day > 31 || Hour > 23 || Minute > 59 || Second > 59)
	{
		return false;
	}

	OutDateTime = FDateTime(FDateTime(Year, Month, Day, Hour, Minute, Second).GetTicks() + FractionTicks - OffsetTicks);
	return true;
}

// ---------------------------------------------------------------------------------------------------------------------
// FEvent

void FEvent::Trigger()
{
	std::lock_guard<std::mutex> Lock(Mutex);
	bTriggered = true;
	if (bIsManualReset)
	{
		Condition.notify_all();
	}
	else
	{
		Condition.notify_one();
	}
}

void FEvent::Reset()
{
	std::lock_guard<std::mutex> Lock(Mutex);
	bTriggered = false;
}

bool FEvent::Wait(uint32 WaitTimeMs, const bool)
{
	std::unique_lock<std::mutex> Lock(Mutex);
	bool bSignaled = true;
	if (WaitTimeMs == MAX_uint32)
	{
		Condition.wait(Lock, [this] { return bTriggered; });
	}
	else
	{
		bSignaled = Condition.wait_for(Lock, std::chrono::milliseconds(WaitTimeMs), [this] { return bTriggered; });
	}
	if (bSignaled && !bIsManualReset)
	{
		bTriggered = false;
	}
	return bSignaled;
}

// ---------------------------------------------------------------------------------------------------------------------
// Threads and tasks

namespace
{
	const std::thread::id GGameThreadId = std::this_thread::get_id();

	struct FGameThreadQueue
	{
		std::mutex Mutex;
		std::deque<TUniqueFunction<void()>> Tasks;
	};

	FGameThreadQueue& GetGameThreadQueue()
	{
		static FGameThreadQueue* Queue = new FGameThreadQueue();
		return *Queue;
	}

	// Runs AnyThread tasks in order on one background thread, like a task graph worker.
	class FBackgroundWorker
	{
	public:
		void Enqueue(TUniqueFunction<void()> Task)
		{
			std::lock_guard<std::mutex> Lock(Mutex);
			if (bStopped)
			{
				return;
			}
			if (!Thread.joinable())
			{
				Thread = std::thread([this] { Run(); });
			}
			Tasks.push_back(std::move(Task));
			Condition.notify_one();
		}

		void Stop()
		{
			{
				std::lock_guard<std::mutex> Lock(Mutex);
				bStopped = true;
				Condition.notify_all();
			}
			if (Thread.joinable())
			{
				Thread.join();
			}
		}

	private:
		void Run()
		{
			while (true)
			{
				TUniqueFunction<void()> Task;
				{
					std::unique_lock<std::mutex> Lock(Mutex);
					Condition.wait(Lock, [this] { return bStopped || !Tasks.empty(); });
					if (Tasks.empty())
					{
						return;
					}
					Task = std::move(Tasks.front());
					Tasks.pop_front();
				}
				Task();
			}
		}

		std::mutex Mutex;
		std::condition_variable Condition;
		std::deque<TUniqueFunction<void()>> Tasks;
		std::thread Thread;
		bool bStopped = false;
	};

	FBackgroundWorker& GetBackgroundWorker()
	{
		static FBackgroundWorker* Worker = new FBackgroundWorker();
		return *Worker;
	}
}

bool IsInGameThread()
{
	return std::this_thread::get_id() == GGameThreadId;
}

void AsyncTask(ENamedThreads::Type Thread, TUniqueFunction<void()> Function)
{
	if (Thread == ENamedThreads::GameThread)
	{
		FGameThreadQueue& Queue = GetGameThreadQueue();
		std::lock_guard<std::mutex> Lock(Queue.Mutex);
		Queue.Tasks.push_back(std::move(Function));
	}
	else
	{
		GetBackgroundWorker().Enqueue(std::move(Function));
	}
}

TFuture<void> UEShim::LaunchAsync(EAsyncExecution, TUniqueFunction<void()> Function)
{
	auto Promise = std::make_shared<std::promise<void>>();
	std::shared_future<void> Future = Promise->get_future().share();
	std::thread([Promise, Task = std::move(Function)]() mutable
		{
			Task();
			Promise->set_value();
		}).detach();
	return TFuture<void>(Future);
}

FTaskGraphInterface& FTaskGraphInterface::Get()
{
	static FTaskGraphInterface Instance;
	return Instance;
}

void FTaskGraphInterface::ProcessThreadUntilIdle(ENamedThreads::Type CurrentThread)
{
	UESHIM_CHECK(CurrentThread == ENamedThreads::GameThread && IsInGameThread());
	bool bDidWork = true;
	while (bDidWork)
	{
		bDidWork = false;
		while (true)
		{
			TUniqueFunction<void()> Task;
			{
				FGameThreadQueue& Queue = GetGameThreadQueue();
				std::lock_guard<std::mutex> Lock(Queue.Mutex);
				if (Queue.Tasks.empty())
				{
					break;
				}
				Task = std::move(Queue.Tasks.front());
				Queue.Tasks.pop_front();
			}
			Task();
			bDidWork = true;
		}
		bDidWork = UEShim::TickHttp() || bDidWork;
	}
}

// ---------------------------------------------------------------------------------------------------------------------
// Platform, paths and files

std::filesystem::path UEShim::ToFsPath(const FString& Path)
{
#if defined(_WIN32)
	return std::filesystem::path(Path.GetStdString());
#else
	return std::filesystem::path(UEShim::ToUtf8(Path.GetStdString()));
#endif
}

namespace
{
	using UEShim::ToFsPath;

	FString FromFsPath(const std::filesystem::path& Path)
	{
		const std::string Utf8 = Path.generic_u8string().empty() ? std::string() : std::string(reinterpret_cast<const char*>(Path.generic_u8string().c_str()));
		return FString(UEShim::FromUtf8(Utf8.data(), Utf8.size()));
	}

	int CurrentProcessId()
	{
#if defined(_WIN32)
		return _getpid();
#else
		return static_cast<int>(getpid());
#endif
	}

	std::filesystem::path& ProjectDirPath()
	{
		static std::filesystem::path* Path = new std::filesystem::path();
		return *Path;
	}

	std::wstring& CommandLineStorage()
	{
		static std::wstring* Storage = new std::wstring();
		return *Storage;
	}
}

FString FPlatformMisc::GetEnvironmentVariable(const TCHAR* VariableName)
{
#if defined(_WIN32)
#if defined(_MSC_VER)
#pragma warning(suppress: 4996)
#endif
	const wchar_t* Value = _wgetenv(VariableName);
	return Value != nullptr ? FString(Value) : FString();
#else
	const char* Value = std::getenv(UEShim::ToUtf8(VariableName).c_str());
	return Value != nullptr ? FString(Value) : FString();
#endif
}

void FPlatformMisc::SetEnvironmentVar(const TCHAR* VariableName, const TCHAR* Value)
{
#if defined(_WIN32)
	_wputenv_s(VariableName, Value != nullptr ? Value : L"");
#else
	if (Value != nullptr && *Value != 0)
	{
		setenv(UEShim::ToUtf8(VariableName).c_str(), UEShim::ToUtf8(Value).c_str(), 1);
	}
	else
	{
		unsetenv(UEShim::ToUtf8(VariableName).c_str());
	}
#endif
}

void FPlatformProcess::Sleep(float Seconds)
{
	std::this_thread::sleep_for(std::chrono::duration<float>(Seconds));
}

namespace
{
	std::mutex& EventPoolMutex()
	{
		static std::mutex* Mutex = new std::mutex();
		return *Mutex;
	}

	// Keeps every pooled event reachable for the lifetime of the process.
	std::vector<FEvent*>& PooledEvents()
	{
		static std::vector<FEvent*>* Events = new std::vector<FEvent*>();
		return *Events;
	}
}

FEvent* FPlatformProcess::GetSynchEventFromPool(bool bIsManualReset)
{
	FEvent* Event = new FEvent(bIsManualReset);
	std::lock_guard<std::mutex> Lock(EventPoolMutex());
	PooledEvents().push_back(Event);
	return Event;
}

void FPlatformProcess::ReturnSynchEventToPool(FEvent*)
{
}

bool FPaths::FileExists(const FString& Path)
{
	std::error_code Error;
	return !Path.IsEmpty() && std::filesystem::is_regular_file(ToFsPath(Path), Error);
}

bool FPaths::DirectoryExists(const FString& Path)
{
	std::error_code Error;
	return !Path.IsEmpty() && std::filesystem::is_directory(ToFsPath(Path), Error);
}

FString FPaths::ProjectDir()
{
	static const FString Dir = []()
	{
		std::error_code Error;
		std::filesystem::path Root = std::filesystem::temp_directory_path(Error) / ("PlayFabGSDKNativeTests-" + std::to_string(CurrentProcessId()));
		std::filesystem::create_directories(Root, Error);
		ProjectDirPath() = Root;
		return FromFsPath(Root) + TEXT("/");
	}();
	return Dir;
}

void FPaths::AppendPath(FString& Path, const FString& Part)
{
	if (Part.IsEmpty())
	{
		return;
	}
	if (Path.IsEmpty())
	{
		Path = Part;
		return;
	}
	const bool bPathEndsWithSeparator = Path.EndsWith(TEXT("/")) || Path.EndsWith(TEXT("\\"));
	const bool bPartStartsWithSeparator = Part[0] == L'/' || Part[0] == L'\\';
	if (bPathEndsWithSeparator && bPartStartsWithSeparator)
	{
		Path += FString(Part.GetStdString().substr(1));
	}
	else if (!bPathEndsWithSeparator && !bPartStartsWithSeparator)
	{
		Path += TEXT("/");
		Path += Part;
	}
	else
	{
		Path += Part;
	}
}

bool IPlatformFile::FileExists(const TCHAR* Filename)
{
	return FPaths::FileExists(Filename);
}

bool IPlatformFile::DirectoryExists(const TCHAR* Directory)
{
	return FPaths::DirectoryExists(Directory);
}

bool IPlatformFile::CreateDirectoryTree(const TCHAR* Directory)
{
	if (Directory == nullptr || *Directory == 0)
	{
		return false;
	}
	std::error_code Error;
	std::filesystem::create_directories(ToFsPath(Directory), Error);
	return FPaths::DirectoryExists(Directory);
}

bool IPlatformFile::DeleteFile(const TCHAR* Filename)
{
	std::error_code Error;
	return std::filesystem::remove(ToFsPath(Filename), Error);
}

FPlatformFileManager& FPlatformFileManager::Get()
{
	static FPlatformFileManager Instance;
	return Instance;
}

bool FFileHelper::LoadFileToString(FString& Result, const TCHAR* Filename)
{
	std::ifstream Stream(ToFsPath(Filename), std::ios::binary);
	if (!Stream)
	{
		return false;
	}
	std::string Bytes((std::istreambuf_iterator<char>(Stream)), std::istreambuf_iterator<char>());
	if (Bytes.size() >= 3 && Bytes.compare(0, 3, "\xEF\xBB\xBF") == 0)
	{
		Bytes.erase(0, 3);
	}
	Result = FString(UEShim::FromUtf8(Bytes.data(), Bytes.size()));
	return true;
}

bool FFileHelper::SaveStringToFile(const FString& String, const TCHAR* Filename, EEncodingOptions)
{
	const std::filesystem::path Path = ToFsPath(Filename);
	std::error_code Error;
	if (Path.has_parent_path())
	{
		std::filesystem::create_directories(Path.parent_path(), Error);
	}
	std::ofstream Stream(Path, std::ios::binary | std::ios::trunc);
	if (!Stream)
	{
		return false;
	}
	const std::string Bytes = UEShim::ToUtf8(String.GetStdString());
	Stream.write(Bytes.data(), static_cast<std::streamsize>(Bytes.size()));
	return static_cast<bool>(Stream);
}

const TCHAR* FCommandLine::Get()
{
	return CommandLineStorage().c_str();
}

void FCommandLine::Set(const TCHAR* NewCommandLine)
{
	CommandLineStorage() = NewCommandLine != nullptr ? NewCommandLine : L"";
}

bool FParse::Param(const TCHAR* Stream, const TCHAR* Param)
{
	std::wistringstream Tokens(Stream != nullptr ? Stream : L"");
	std::wstring Token;
	while (Tokens >> Token)
	{
		if (Token.size() > 1 && (Token[0] == L'-' || Token[0] == L'/') && FString(Token.substr(1)).Equals(FString(Param), ESearchCase::IgnoreCase))
		{
			return true;
		}
	}
	return false;
}

// ---------------------------------------------------------------------------------------------------------------------
// Modules and engine lifetime

FModuleManager& FModuleManager::Get()
{
	static FModuleManager* Instance = new FModuleManager();
	return *Instance;
}

void FModuleManager::RegisterModule(const TCHAR* ModuleName, FModuleFactory Factory)
{
	std::lock_guard<std::recursive_mutex> Lock(Mutex);
	FModuleEntry Entry;
	Entry.Name = ModuleName;
	Entry.Factory = Factory;
	Modules.push_back(std::move(Entry));
}

IModuleInterface& FModuleManager::LoadModuleCheckedInternal(const TCHAR* ModuleName)
{
	std::lock_guard<std::recursive_mutex> Lock(Mutex);
	for (FModuleEntry& Entry : Modules)
	{
		if (Entry.Name == ModuleName)
		{
			if (!Entry.Module)
			{
				Entry.Module.reset(Entry.Factory());
				Entry.Module->StartupModule();
			}
			return *Entry.Module;
		}
	}
	UEShim::CheckFailed("LoadModuleChecked: module is not registered (missing IMPLEMENT_MODULE?)", __FILE__, __LINE__);
}

void FModuleManager::UnloadModulesAtShutdown()
{
	std::vector<std::unique_ptr<IModuleInterface>> Loaded;
	{
		std::lock_guard<std::recursive_mutex> Lock(Mutex);
		for (auto It = Modules.rbegin(); It != Modules.rend(); ++It)
		{
			if (It->Module)
			{
				Loaded.push_back(std::move(It->Module));
			}
		}
	}
	for (std::unique_ptr<IModuleInterface>& Module : Loaded)
	{
		Module->ShutdownModule();
		Module.reset();
	}
}

FUrlConfig FURL::UrlConfig;

void UEShim::ShutdownEngine()
{
	GetBackgroundWorker().Stop();
	FModuleManager::Get().UnloadModulesAtShutdown();
	if (!ProjectDirPath().empty())
	{
		std::error_code Error;
		std::filesystem::remove_all(ProjectDirPath(), Error);
	}
}
