// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PlayfabGSDK : ModuleRules
{
	public PlayfabGSDK(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Core",
				"Json",
				"HTTP"
				// ... add other public dependencies that you statically link with here ...
			}
			);
	}
}
