// Copyright 2021 Stefan Krismann
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// 	http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
