// Copyright (C) Microsoft Corporation. All rights reserved.

// Declarations shared by the shim's implementation files; not part of the emulated UE API.

#pragma once

#include <filesystem>

#include "UEShim/UEShim.h"

namespace UEShim
{
	// Offers a log line to the automation test that is running, if any. Returns true if the test expected it.
	bool CaptureLogForRunningTest(const FString& Message, ELogVerbosity::Type Verbosity, const TCHAR* Category);

	std::filesystem::path ToFsPath(const FString& Path);
}
