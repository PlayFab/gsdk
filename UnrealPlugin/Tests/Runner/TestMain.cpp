// Copyright (C) Microsoft Corporation. All rights reserved.

// Runs the automation specs compiled into this executable, like "Automation RunTests <filter>" does in the editor.
// Usage: <executable> [filter...]   (a test runs if its full name contains any of the filters)

#include "UEShim/UEShim.h"

int main(int argc, char** argv)
{
	std::vector<FString> Filters;
	for (int Index = 1; Index < argc; ++Index)
	{
		Filters.push_back(FString(argv[Index]));
	}

	const int ExitCode = UEShim::RunAutomationTests(Filters);
	UEShim::ShutdownEngine();
	return ExitCode;
}
