// Copyright (C) Microsoft Corporation. All rights reserved.

// Tests of the shim itself, for Unreal behavior that the plugin's tests rely on. If the shim accepted input that Unreal
// rejects (or the reverse), a plugin test could pass here and fail in the editor.

#include "CoreMinimal.h"
#include "Json.h"
#include "Misc/AutomationTest.h"

BEGIN_DEFINE_SPEC(FUEShimJsonSpec, "UEShim.Json", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter)
bool DeserializeNumber(const TCHAR* Number, double& OutValue);
END_DEFINE_SPEC(FUEShimJsonSpec)

// Deserializes {"Value":<Number>} the way the plugin deserializes agent responses.
bool FUEShimJsonSpec::DeserializeNumber(const TCHAR* Number, double& OutValue)
{
	TSharedPtr<FJsonObject> Object;
	if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(FString::Printf(TEXT("{\"Value\":%s}"), Number)), Object))
	{
		return false;
	}
	OutValue = Object->GetNumberField(TEXT("Value"));
	return true;
}

void FUEShimJsonSpec::Define()
{
	// Like Unreal's reader, the shim accepts only numbers that follow the JSON grammar (RFC 8259).
	Describe("Numbers", [this]()
		{
			It("RejectsMalformedNumbers", [this]()
				{
					const TCHAR* const MalformedNumbers[] = {
						TEXT("01"), TEXT("-01"), TEXT("00"), TEXT("1."), TEXT("1.e5"), TEXT(".5"), TEXT("+1"), TEXT("-"),
						TEXT("1e"), TEXT("1E+"), TEXT("1e-"), TEXT("1.5.3"), TEXT("0x10"), TEXT("NaN"), TEXT("Infinity") };
					for (const TCHAR* Number : MalformedNumbers)
					{
						double Value = 0.0;
						TestFalse(FString::Printf(TEXT("Verify %s is rejected."), Number), DeserializeNumber(Number, Value));
					}
				});

			It("ParsesValidNumbers", [this]()
				{
					struct FValidNumber
					{
						const TCHAR* Json;
						double Value;
					};
					const FValidNumber ValidNumbers[] = {
						{ TEXT("0"), 0.0 }, { TEXT("-0"), 0.0 }, { TEXT("10"), 10.0 }, { TEXT("-12"), -12.0 },
						{ TEXT("0.25"), 0.25 }, { TEXT("-1.5"), -1.5 }, { TEXT("1e3"), 1000.0 }, { TEXT("1E+3"), 1000.0 },
						{ TEXT("2.5e-1"), 0.25 }, { TEXT("1e03"), 1000.0 } };
					for (const FValidNumber& Number : ValidNumbers)
					{
						double Value = 0.0;
						if (TestTrue(FString::Printf(TEXT("Verify %s is accepted."), Number.Json), DeserializeNumber(Number.Json, Value)))
						{
							TestEqual(FString::Printf(TEXT("Verify the value of %s."), Number.Json), Value, Number.Value);
						}
					}
				});
		});
}
