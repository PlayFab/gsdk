# PlayfabGSDK_Unreal
This Unreal Plugin is implementing the GSDK directly in Unreal Engine.

It was tested with Unreal Engine 4.26.2, but should work with other engines as well.

# Setup

## Dependencies
When using the plugin a few things need to be taken care off.

First, when you are building dedicated server, make sure you add the following to DefaultGame.ini

```ini
[/Script/UnrealEd.ProjectPackagingSettings]
IncludeAppLocalPrerequisites=True
```

If the category already exists in your DefaultGame.ini, then just add the second line to it. This ensures that all app local dependencies ship with the game as well.

If you are using Continuous Integration (CI), then you could add it to your setup to only turn this flag on when building a dedicated server, so the additional dlls only get added if it is a dedicated server build.

## Disabling plugins
When you add Dedicated Server support to your project, you will definitely have created a \<projectname\>Server.Target.cs file.

In this file add the following lines to the constructor:
```csharp
DisablePlugins.Add("WMFMediaPlayer");
DisablePlugins.Add("AsyncLoadingScreen"); //if you are using this plugin
DisablePlugins.Add("WindowsMoviePlayer");
DisablePlugins.Add("MediaFoundationMediaPlayer");
```

Result would be:
```csharp
public class <projectname>ServerTarget : TargetRules
{
	public <projectname>ServerTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange( new string[] { "<projectname>" } );
		
		DisablePlugins.Add("WMFMediaPlayer");
		DisablePlugins.Add("AsyncLoadingScreen"); //if you are using this plugin
		DisablePlugins.Add("WindowsMoviePlayer");
		DisablePlugins.Add("MediaFoundationMediaPlayer");
	}
}

```