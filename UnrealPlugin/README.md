# Instructions on Integrating the PlayFab GSDK Unreal Plugin Preview

This Unreal Plugin is implementing the GSDK directly in Unreal Engine.

This plugin offers both a Blueprint API and a C++ API. This does however not mean that the Blueprint API does not require the game to be a C++ project. If it is currently a Blueprint only project, then you need to convert it beforehand, before adding the plugin.

The following nodes are supported:
![PlayFab GSDK Blueprint Nodes](Documentation/GSDKBlueprintNodes.png)

It was tested with Unreal Engine 4.26.2, but should work with other engines as well.

The sample game these instructions were created with was called ThirdPersonMP, so replace anywhere you see that with your game name.

## Requirements

* Download Visual Studio (the [community version is free](https://visualstudio.microsoft.com/vs/community/))
	- Requires Workloads: .NET desktop development, and Desktop development with C++
* Download Unreal Engine Source Build by following [these instructions](https://docs.unrealengine.com/4.26/ProgrammingAndScripting/ProgrammingWithCPP/DownloadingSourceCode/) from the Unreal Engine website. This was tested on [4.26.2](https://github.com/EpicGames/UnrealEngine/releases/tag/4.26.2-release) (Link requires permissions)
* Download the [Unreal PlayFabGSDK Plugin folder](https://github.com/PlayFab/gsdk/tree/master/UnrealPlugin)
* [Optional] Download the [LocalMultiplayerAgent](https://github.com/PlayFab/MpsAgent/releases)
	* [Optional] Alternately, download the [LocalMultiplayerAgent source](https://github.com/PlayFab/MpsAgent/tree/main/LocalMultiplayerAgent)
* [Optional] [PlayFab Marketplace plugin](https://www.unrealengine.com/marketplace/product/playfab-sdk) or the [source version on GitHub](https://github.com/PlayFab/UnrealMarketplacePlugin/tree/master/4.26/PlayFabPlugin/PlayFab) (Not required for GSDK, but required for most other PlayFab services, including login)

## Project creation

Unreal GSDK is installed into an Unreal server project. You will need a network-enabled multiplayer Unreal project with a dedicated-server mode. If you do not have one, you can follow our [Unreal prerequsitie setup](ThirdPersonMPSetup.md) guide. Once you have a network-enabled, multiplayer game, with a dedicated server, return here and continue.

## Project GSDK setup

With a multiplayer project configured for both a network connected game-client and game-server, you're ready to [install the GSDK](ThirdPersonMPGSDKSetup.md). This will allow your server to run on PlayFab Multiplayer Services.

## Unreal Project Build Configurations

Once the GSDK is installed and configured in your project, you can [build your Unreal project](ThirdPersonMPBuild.md). There are multiple build configurations you will need to execute the full range of tests in future steps.

## Test Local Server Deployment

Once you have any client build, and a "Development Server" build, you can [test the MPS deployment process locally](ThirdPersonMPLocalDeploy.md), and verify your server works.

## Deploy to PlayFab

Finally, you're ready to [deploy your game-server to PlayFab](ThirdPersonMPCloudDeploy.md), and test with real game servers in the cloud.

# Acknowledgements

Thank you to [Nick Arthur](https://github.com/narthur157) for his amazing [Dockerfile](https://github.com/narthur157/playfab-gsdk-ue4).

Thank you to [Adam Rehn](https://github.com/adamrehn) for all his work with Unreal and Docker, including code, samples, examples, and tutorials.
