// Copyright (C) Microsoft Corporation. All rights reserved.

// Emulation of the Unreal automation spec framework (Misc/AutomationTest.h): BEGIN_DEFINE_SPEC / Describe / It /
// BeforeEach / AfterEach, the TestXxx assertions and AddExpectedError/AddExpectedMessage. As in Unreal, a test fails
// if it records an error, if an unexpected error is logged while it runs, or if an expected message is not logged
// exactly the expected number of times. Expected messages are matched case-insensitively against the log text, and a
// log line is consumed by the first matching expectation. Unexpected warnings are reported but don't fail the test.

#pragma once

#include "UEShim/Core.h"

#include <cmath>

namespace EAutomationTestFlags
{
	enum Type : uint32
	{
		EditorContext = 0x00000001,
		ClientContext = 0x00000002,
		ServerContext = 0x00000004,
		CommandletContext = 0x00000008,
		ApplicationContextMask = EditorContext | ClientContext | ServerContext | CommandletContext,
		SmokeFilter = 0x01000000,
		EngineFilter = 0x02000000,
		ProductFilter = 0x04000000,
		PerfFilter = 0x08000000,
		StressFilter = 0x10000000,
		NegativeFilter = 0x20000000,
		FilterMask = SmokeFilter | EngineFilter | ProductFilter | PerfFilter | StressFilter | NegativeFilter,
	};
}

namespace EAutomationExpectedErrorFlags
{
	enum MatchType
	{
		Exact,
		Contains,
	};
}

namespace EAutomationExpectedMessageFlags
{
	enum MatchType
	{
		Exact,
		Contains,
	};
}

namespace UEShim
{
	template <typename T>
	inline constexpr bool IsStringLike = std::is_same_v<std::decay_t<T>, FString>
		|| std::is_same_v<std::decay_t<T>, const TCHAR*> || std::is_same_v<std::decay_t<T>, TCHAR*>
		|| std::is_same_v<std::decay_t<T>, const ANSICHAR*> || std::is_same_v<std::decay_t<T>, ANSICHAR*>;

	template <typename T>
	inline constexpr bool IsPlainInteger = std::is_integral_v<T> && !std::is_same_v<T, bool>
		&& !std::is_same_v<T, char> && !std::is_same_v<T, wchar_t> && !std::is_same_v<T, char16_t> && !std::is_same_v<T, char32_t>;

	template <typename ActualType, typename ExpectedType>
	bool AreTestValuesEqual(const ActualType& Actual, const ExpectedType& Expected)
	{
		if constexpr (IsStringLike<ActualType> && IsStringLike<ExpectedType>)
		{
			// Unreal's string TestEqual is case-sensitive.
			return FString(Actual).Equals(FString(Expected), ESearchCase::CaseSensitive);
		}
		else if constexpr (IsPlainInteger<ActualType> && IsPlainInteger<ExpectedType>)
		{
			return std::cmp_equal(Actual, Expected);
		}
		else if constexpr (std::is_arithmetic_v<ActualType> && std::is_arithmetic_v<ExpectedType>)
		{
			return std::abs(static_cast<double>(Actual) - static_cast<double>(Expected)) <= UE_KINDA_SMALL_NUMBER;
		}
		else
		{
			return Actual == Expected;
		}
	}

	template <typename T>
	FString ToTestString(const T& Value)
	{
		if constexpr (IsStringLike<T>)
		{
			return FString(TEXT("\"")) + FString(Value) + TEXT("\"");
		}
		else if constexpr (std::is_same_v<T, bool>)
		{
			return Value ? TEXT("true") : TEXT("false");
		}
		else if constexpr (std::is_arithmetic_v<T>)
		{
			return FString(std::to_wstring(Value));
		}
		else if constexpr (std::is_enum_v<T>)
		{
			return FString(std::to_wstring(static_cast<long long>(Value)));
		}
		else
		{
			return TEXT("<value>");
		}
	}

	struct FExpectedMessage;
	struct FSpecScope;
}

class FAutomationTestBase
{
public:
	FAutomationTestBase(const FString& InName, bool bInComplexTask);
	virtual ~FAutomationTestBase();

	virtual uint32 GetTestFlags() const = 0;
	const FString& GetTestName() const { return TestName; }

	void AddError(const FString& InError, int32 StackOffset = 0);
	void AddWarning(const FString& InWarning, int32 StackOffset = 0);
	void AddInfo(const FString& InLogItem, int32 StackOffset = 0);

	void AddExpectedError(FString ExpectedPatternString, EAutomationExpectedErrorFlags::MatchType CompareType = EAutomationExpectedErrorFlags::Contains, int32 Occurrences = 1, bool IsRegex = true);
	void AddExpectedMessage(FString ExpectedPatternString, EAutomationExpectedMessageFlags::MatchType CompareType = EAutomationExpectedMessageFlags::Contains, int32 Occurrences = 1, bool IsRegex = true);
	void AddExpectedMessage(FString ExpectedPatternString, ELogVerbosity::Type ExpectedVerbosity, EAutomationExpectedMessageFlags::MatchType CompareType = EAutomationExpectedMessageFlags::Contains, int32 Occurrences = 1, bool IsRegex = true);

	bool TestTrue(const FString& What, bool Value);
	bool TestFalse(const FString& What, bool Value);

	template <typename ActualType, typename ExpectedType>
	bool TestEqual(const FString& What, const ActualType& Actual, const ExpectedType& Expected)
	{
		if (UEShim::AreTestValuesEqual(Actual, Expected))
		{
			return true;
		}
		AddError(FString::Printf(TEXT("Expected '%s' to be %s, but it was %s."), *What, *UEShim::ToTestString(Expected), *UEShim::ToTestString(Actual)));
		return false;
	}

	template <typename ActualType, typename ExpectedType>
	bool TestNotEqual(const FString& What, const ActualType& Actual, const ExpectedType& Expected)
	{
		if (!UEShim::AreTestValuesEqual(Actual, Expected))
		{
			return true;
		}
		AddError(FString::Printf(TEXT("Expected '%s' to differ from %s, but it was %s."), *What, *UEShim::ToTestString(Expected), *UEShim::ToTestString(Actual)));
		return false;
	}

	template <typename PointerType>
	bool TestValid(const FString& What, const PointerType& Pointer)
	{
		return TestTrue(What, Pointer.IsValid());
	}

	template <typename PointerType>
	bool TestNotNull(const FString& What, const PointerType* Pointer)
	{
		return TestTrue(What, Pointer != nullptr);
	}

	// Used by the shim's log capture and test runner.
	void BeginRun();
	bool EndRun();
	// Returns true if the message matched one of the test's expected messages.
	bool HandleLogMessage(const FString& Message, ELogVerbosity::Type Verbosity, const TCHAR* Category);
	virtual void CollectTestCases(std::vector<std::pair<FString, std::vector<TFunction<void()>>>>& OutTestCases) = 0;

private:
	FString TestName;
	std::recursive_mutex Mutex;
	std::vector<FString> Errors;
	std::vector<FString> Warnings;
	std::vector<std::unique_ptr<UEShim::FExpectedMessage>> ExpectedMessages;
};

class FAutomationSpecBase : public FAutomationTestBase
{
public:
	FAutomationSpecBase(const FString& InName, bool bInComplexTask);
	~FAutomationSpecBase() override;

	void Describe(const FString& InDescription, TFunction<void()> DoWork);
	void xDescribe(const FString&, TFunction<void()>) {}
	void It(const FString& InDescription, TFunction<void()> DoWork);
	void xIt(const FString&, TFunction<void()>) {}
	void BeforeEach(TFunction<void()> DoWork);
	void AfterEach(TFunction<void()> DoWork);

	void CollectTestCases(std::vector<std::pair<FString, std::vector<TFunction<void()>>>>& OutTestCases) override;

protected:
	virtual void Define() = 0;
	virtual FString GetBeautifiedTestName() const = 0;

private:
	std::unique_ptr<UEShim::FSpecScope> RootScope;
	UEShim::FSpecScope* CurrentScope = nullptr;
};

#define BEGIN_DEFINE_SPEC(TClass, PrettyName, TFlags) \
	class TClass : public FAutomationSpecBase \
	{ \
	public: \
		TClass(const FString& InName) : FAutomationSpecBase(InName, false) \
		{ \
			static_assert(((TFlags) & EAutomationTestFlags::ApplicationContextMask) != 0, "AutomationTest has no application flag. It shouldn't run."); \
		} \
		virtual uint32 GetTestFlags() const override { return TFlags; } \
	protected: \
		virtual FString GetBeautifiedTestName() const override { return PrettyName; } \
		virtual void Define() override;

#define END_DEFINE_SPEC(TClass) \
	}; \
	namespace \
	{ \
		TClass TClass##AutomationSpecInstance(TEXT(#TClass)); \
	}

namespace UEShim
{
	// Runs every registered test whose full name ("<spec name>.<Describe>.<It>") contains one of the filters (all tests
	// if there are none). Returns the process exit code.
	int RunAutomationTests(const std::vector<FString>& Filters);
}
