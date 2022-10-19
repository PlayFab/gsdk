// Copyright (C) Microsoft Corporation. All rights reserved.

using UnrealBuildTool;

public class PlayFabGSDK : ModuleRules
{
	public PlayFabGSDK(ReadOnlyTargetRules Target) : base(Target)
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
			
			if (Target.Type == TargetType.Server)
			{
				if (Target.Platform == UnrealTargetPlatform.Win64)
				{
					RuntimeDependencies.Add("$(TargetOutputDir)/D3DCompiler_43.dll", "C:/Windows/System32/D3DCompiler_43.dll", StagedFileType.SystemNonUFS);
				}
			}
	}
}
