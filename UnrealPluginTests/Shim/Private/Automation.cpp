// Copyright (C) Microsoft Corporation. All rights reserved.

#include <algorithm>
#include <cstdio>
#include <functional>
#include <regex>
#include <vector>

#include "ShimInternal.h"

namespace UEShim
{
	struct FExpectedMessage
	{
		FString Pattern;
		ELogVerbosity::Type Verbosity = ELogVerbosity::All;
		bool bExactMatch = false;
		bool bIsRegex = true;
		int32 ExpectedOccurrences = 1;
		int32 ActualOccurrences = 0;
		std::wregex Regex;

		bool Matches(const FString& Message) const
		{
			if (bIsRegex)
			{
				return bExactMatch ? std::regex_match(Message.GetStdString(), Regex) : std::regex_search(Message.GetStdString(), Regex);
			}
			return bExactMatch ? Message.Equals(Pattern, ESearchCase::IgnoreCase) : Message.Contains(Pattern, ESearchCase::IgnoreCase);
		}
	};

	struct FSpecScope
	{
		FString Description;
		std::vector<TFunction<void()>> BeforeEach;
		std::vector<TFunction<void()>> AfterEach;
		std::vector<std::pair<FString, TFunction<void()>>> Its;
		std::vector<std::unique_ptr<FSpecScope>> Children;
	};
}

namespace
{
	std::vector<FAutomationTestBase*>& RegisteredTests()
	{
		static std::vector<FAutomationTestBase*>* Tests = new std::vector<FAutomationTestBase*>();
		return *Tests;
	}

	std::mutex& RunningTestMutex()
	{
		static std::mutex* Mutex = new std::mutex();
		return *Mutex;
	}

	FAutomationTestBase*& RunningTest()
	{
		static FAutomationTestBase* Test = nullptr;
		return Test;
	}

	void PrintLine(const FString& Line)
	{
		std::printf("%s\n", UEShim::ToUtf8(Line.GetStdString()).c_str());
		std::fflush(stdout);
	}
}

bool UEShim::CaptureLogForRunningTest(const FString& Message, ELogVerbosity::Type Verbosity, const TCHAR* Category)
{
	std::lock_guard<std::mutex> Lock(RunningTestMutex());
	return RunningTest() != nullptr && RunningTest()->HandleLogMessage(Message, Verbosity, Category);
}

// ---------------------------------------------------------------------------------------------------------------------
// FAutomationTestBase

FAutomationTestBase::FAutomationTestBase(const FString& InName, bool) : TestName(InName)
{
	RegisteredTests().push_back(this);
}

FAutomationTestBase::~FAutomationTestBase()
{
	std::vector<FAutomationTestBase*>& Tests = RegisteredTests();
	Tests.erase(std::remove(Tests.begin(), Tests.end(), this), Tests.end());
}

void FAutomationTestBase::AddError(const FString& InError, int32)
{
	std::lock_guard<std::recursive_mutex> Lock(Mutex);
	Errors.push_back(InError);
}

void FAutomationTestBase::AddWarning(const FString& InWarning, int32)
{
	std::lock_guard<std::recursive_mutex> Lock(Mutex);
	Warnings.push_back(InWarning);
}

void FAutomationTestBase::AddInfo(const FString& InLogItem, int32)
{
	PrintLine(FString(TEXT("    Info: ")) + InLogItem);
}

void FAutomationTestBase::AddExpectedError(FString ExpectedPatternString, EAutomationExpectedErrorFlags::MatchType CompareType, int32 Occurrences, bool IsRegex)
{
	// Like Unreal, an expected "error" also matches warnings.
	AddExpectedMessage(ExpectedPatternString, ELogVerbosity::Warning, static_cast<EAutomationExpectedMessageFlags::MatchType>(CompareType), Occurrences, IsRegex);
}

void FAutomationTestBase::AddExpectedMessage(FString ExpectedPatternString, EAutomationExpectedMessageFlags::MatchType CompareType, int32 Occurrences, bool IsRegex)
{
	AddExpectedMessage(ExpectedPatternString, ELogVerbosity::All, CompareType, Occurrences, IsRegex);
}

void FAutomationTestBase::AddExpectedMessage(FString ExpectedPatternString, ELogVerbosity::Type ExpectedVerbosity, EAutomationExpectedMessageFlags::MatchType CompareType, int32 Occurrences, bool IsRegex)
{
	std::lock_guard<std::recursive_mutex> Lock(Mutex);
	if (Occurrences < 0)
	{
		Errors.push_back(FString::Printf(TEXT("Adding expected message matching '%s' failed: number of expected occurrences must be >= 0"), *ExpectedPatternString));
		return;
	}
	for (const std::unique_ptr<UEShim::FExpectedMessage>& Existing : ExpectedMessages)
	{
		if (Existing->Pattern == ExpectedPatternString)
		{
			Warnings.push_back(FString::Printf(TEXT("Adding expected message matching '%s' failed: cannot add duplicate entries"), *ExpectedPatternString));
			return;
		}
	}

	auto Expected = std::make_unique<UEShim::FExpectedMessage>();
	Expected->Pattern = ExpectedPatternString;
	Expected->Verbosity = ExpectedVerbosity;
	Expected->bExactMatch = CompareType == EAutomationExpectedMessageFlags::Exact;
	Expected->bIsRegex = IsRegex;
	Expected->ExpectedOccurrences = Occurrences;
	if (IsRegex)
	{
		try
		{
			Expected->Regex = std::wregex(ExpectedPatternString.GetStdString(), std::regex_constants::ECMAScript | std::regex_constants::icase);
		}
		catch (const std::regex_error&)
		{
			Errors.push_back(FString::Printf(TEXT("Expected message pattern '%s' is not a valid regular expression"), *ExpectedPatternString));
			return;
		}
	}
	ExpectedMessages.push_back(std::move(Expected));
}

bool FAutomationTestBase::TestTrue(const FString& What, bool Value)
{
	if (!Value)
	{
		AddError(FString::Printf(TEXT("Expected '%s' to be true."), *What));
	}
	return Value;
}

bool FAutomationTestBase::TestFalse(const FString& What, bool Value)
{
	if (Value)
	{
		AddError(FString::Printf(TEXT("Expected '%s' to be false."), *What));
	}
	return !Value;
}

void FAutomationTestBase::BeginRun()
{
	std::lock_guard<std::recursive_mutex> Lock(Mutex);
	Errors.clear();
	Warnings.clear();
	ExpectedMessages.clear();
}

bool FAutomationTestBase::HandleLogMessage(const FString& Message, ELogVerbosity::Type Verbosity, const TCHAR* Category)
{
	std::lock_guard<std::recursive_mutex> Lock(Mutex);
	for (const std::unique_ptr<UEShim::FExpectedMessage>& Expected : ExpectedMessages)
	{
		if (Verbosity <= Expected->Verbosity && Expected->Matches(Message))
		{
			++Expected->ActualOccurrences;
			return true;
		}
	}
	if (Verbosity <= ELogVerbosity::Error)
	{
		Errors.push_back(FString(Category) + TEXT(": ") + Message);
	}
	else if (Verbosity == ELogVerbosity::Warning)
	{
		Warnings.push_back(FString(Category) + TEXT(": ") + Message);
	}
	return false;
}

bool FAutomationTestBase::EndRun()
{
	std::lock_guard<std::recursive_mutex> Lock(Mutex);
	for (const std::unique_ptr<UEShim::FExpectedMessage>& Expected : ExpectedMessages)
	{
		if (Expected->ExpectedOccurrences > 0 && Expected->ActualOccurrences != Expected->ExpectedOccurrences)
		{
			Errors.push_back(FString::Printf(TEXT("Expected message matching pattern '%s' to occur %d time(s), but it was found %d time(s)."),
				*Expected->Pattern, Expected->ExpectedOccurrences, Expected->ActualOccurrences));
		}
		else if (Expected->ExpectedOccurrences == 0 && Expected->ActualOccurrences == 0)
		{
			Errors.push_back(FString::Printf(TEXT("Expected message matching pattern '%s' to occur at least once, but it was not found."), *Expected->Pattern));
		}
	}
	for (const FString& Warning : Warnings)
	{
		PrintLine(FString(TEXT("    Warning: ")) + Warning);
	}
	for (const FString& Error : Errors)
	{
		PrintLine(FString(TEXT("    Error: ")) + Error);
	}
	return Errors.empty();
}

// ---------------------------------------------------------------------------------------------------------------------
// FAutomationSpecBase

FAutomationSpecBase::FAutomationSpecBase(const FString& InName, bool bInComplexTask) : FAutomationTestBase(InName, bInComplexTask)
{
}

FAutomationSpecBase::~FAutomationSpecBase() = default;

void FAutomationSpecBase::Describe(const FString& InDescription, TFunction<void()> DoWork)
{
	UESHIM_CHECK(CurrentScope != nullptr);
	auto Child = std::make_unique<UEShim::FSpecScope>();
	Child->Description = InDescription;
	UEShim::FSpecScope* const Parent = CurrentScope;
	CurrentScope = Child.get();
	Parent->Children.push_back(std::move(Child));
	DoWork();
	CurrentScope = Parent;
}

void FAutomationSpecBase::It(const FString& InDescription, TFunction<void()> DoWork)
{
	UESHIM_CHECK(CurrentScope != nullptr);
	CurrentScope->Its.emplace_back(InDescription, std::move(DoWork));
}

void FAutomationSpecBase::BeforeEach(TFunction<void()> DoWork)
{
	UESHIM_CHECK(CurrentScope != nullptr);
	CurrentScope->BeforeEach.push_back(std::move(DoWork));
}

void FAutomationSpecBase::AfterEach(TFunction<void()> DoWork)
{
	UESHIM_CHECK(CurrentScope != nullptr);
	CurrentScope->AfterEach.push_back(std::move(DoWork));
}

void FAutomationSpecBase::CollectTestCases(std::vector<std::pair<FString, std::vector<TFunction<void()>>>>& OutTestCases)
{
	if (!RootScope)
	{
		RootScope = std::make_unique<UEShim::FSpecScope>();
		CurrentScope = RootScope.get();
		Define();
		CurrentScope = nullptr;
	}

	// Each It runs the BeforeEach blocks from the outermost scope inwards, then the It, then the AfterEach blocks from
	// the innermost scope outwards. Test names join the Describe/It descriptions with dots, as Unreal does.
	std::vector<const UEShim::FSpecScope*> Path;
	std::function<void(const UEShim::FSpecScope&)> Visit = [&](const UEShim::FSpecScope& Scope)
	{
		Path.push_back(&Scope);
		for (const auto& It : Scope.Its)
		{
			FString Name = GetBeautifiedTestName();
			std::vector<TFunction<void()>> Steps;
			for (const UEShim::FSpecScope* Ancestor : Path)
			{
				if (!Ancestor->Description.IsEmpty())
				{
					Name += TEXT(".");
					Name += Ancestor->Description;
				}
				Steps.insert(Steps.end(), Ancestor->BeforeEach.begin(), Ancestor->BeforeEach.end());
			}
			Name += TEXT(".");
			Name += It.first;
			Steps.push_back(It.second);
			for (auto Ancestor = Path.rbegin(); Ancestor != Path.rend(); ++Ancestor)
			{
				Steps.insert(Steps.end(), (*Ancestor)->AfterEach.begin(), (*Ancestor)->AfterEach.end());
			}
			OutTestCases.emplace_back(Name, std::move(Steps));
		}
		for (const std::unique_ptr<UEShim::FSpecScope>& Child : Scope.Children)
		{
			Visit(*Child);
		}
		Path.pop_back();
	};
	Visit(*RootScope);
}

// ---------------------------------------------------------------------------------------------------------------------
// Runner

int UEShim::RunAutomationTests(const std::vector<FString>& Filters)
{
	struct FTestCase
	{
		FAutomationTestBase* Test;
		FString Name;
		std::vector<TFunction<void()>> Steps;
	};

	std::vector<FTestCase> TestCases;
	const std::vector<FAutomationTestBase*> Tests = RegisteredTests();
	for (FAutomationTestBase* Test : Tests)
	{
		std::vector<std::pair<FString, std::vector<TFunction<void()>>>> Cases;
		Test->CollectTestCases(Cases);
		for (auto& Case : Cases)
		{
			const bool bSelected = Filters.empty() || std::any_of(Filters.begin(), Filters.end(), [&](const FString& Filter) { return Case.first.Contains(Filter); });
			if (bSelected)
			{
				TestCases.push_back({ Test, Case.first, std::move(Case.second) });
			}
		}
	}

	if (TestCases.empty())
	{
		PrintLine(TEXT("No tests matched the filter."));
		return 1;
	}

	// Run from the scratch project directory, like an editor process, so relative paths that the plugin creates
	// (e.g. the log folder of the test configuration) end up there and are cleaned up at shutdown.
	std::error_code DirectoryError;
	const std::filesystem::path OriginalDirectory = std::filesystem::current_path(DirectoryError);
	std::filesystem::current_path(UEShim::ToFsPath(FPaths::ProjectDir()), DirectoryError);

	std::vector<FString> FailedTests;
	for (FTestCase& Case : TestCases)
	{
		PrintLine(FString(TEXT("[ RUN      ] ")) + Case.Name);
		Case.Test->BeginRun();
		{
			std::lock_guard<std::mutex> Lock(RunningTestMutex());
			RunningTest() = Case.Test;
		}
		for (const TFunction<void()>& Step : Case.Steps)
		{
			Step();
		}
		{
			std::lock_guard<std::mutex> Lock(RunningTestMutex());
			RunningTest() = nullptr;
		}
		if (Case.Test->EndRun())
		{
			PrintLine(FString(TEXT("[       OK ] ")) + Case.Name);
		}
		else
		{
			PrintLine(FString(TEXT("[  FAILED  ] ")) + Case.Name);
			FailedTests.push_back(Case.Name);
		}
	}

	PrintLine(FString::Printf(TEXT("%d test(s) run, %d passed, %d failed."), static_cast<int32>(TestCases.size()),
		static_cast<int32>(TestCases.size() - FailedTests.size()), static_cast<int32>(FailedTests.size())));
	for (const FString& Failed : FailedTests)
	{
		PrintLine(FString(TEXT("  FAILED: ")) + Failed);
	}

	std::filesystem::current_path(OriginalDirectory, DirectoryError);
	return FailedTests.empty() ? 0 : 1;
}
