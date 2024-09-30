// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;
using System;

public class SlateUGSServerTarget : TargetRules
{
	public SlateUGSServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.Latest;

		ExtraModuleNames.Add("SlateUGS");
	}
}