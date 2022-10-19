// Copyright (C) Microsoft Corporation. All rights reserved.

#include "GSDKInternalUtils.h"

#include "HAL/PlatformMisc.h"

FString UGSDKInternalUtils::GetEnvironmentVariable(const FString& EnvironmentVariableName)
{
	return FPlatformMisc::GetEnvironmentVariable(*EnvironmentVariableName);
}
