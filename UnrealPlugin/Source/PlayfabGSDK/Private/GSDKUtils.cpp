// Copyright Stefan Krismann
// MIT License

#include "GSDKUtils.h"

#if PLATFORM_WINDOWS
#include "Windows/WindowsPlatformMisc.h"
#elif PLATFORM_UNIX
#include "Unix/UnixPlatformMisc.h"
#endif

FString UGSDKUtils::GetEnvironmentVariable(const FString& EnvironmentVariableName)
{
#if PLATFORM_WINDOWS
	return FWindowsPlatformMisc::GetEnvironmentVariable(*EnvironmentVariableName);
#elif PLATFORM_UNIX
	return FUnixPlatformMisc::GetEnvironmentVariable(*EnvironmentVariableName);
#else
	return TEXT("");
#endif
}
