# How to use PlayFab GSDK Unreal plugin

This article describes how to integrate PlayFab Multiplayer Server SDK (GSDK) Unreal Online Subsystem (OSS) plugin with your Unreal game project.

This plugin offers both a Blueprint API and a C++ API. The Blueprint API still requires your Unreal Project to be a C++ project, and will not work on a Blueprint only project. If it is currently a Blueprint only project, then you need to convert to a C++ project beforehand, before adding the plugin.


**The implementation has been tested for the following versions: 4.26, 4.27, 5 - early access and 5 - official release.**

The sample game these instructions were created with was called ThirdPersonMP, so replace anywhere you see that with your game name.

## Requirements

* Download Visual Studio. The [community version](https://visualstudio.microsoft.com/vs/community/) is free.
	* Required workloads: .NET desktop development and Desktop development with C++
* Download Unreal Engine Source Code. For instructions, see [Downloading Unreal Engine Source code (external)](https://docs.unrealengine.com/4.26/ProgrammingAndScripting/ProgrammingWithCPP/DownloadingSourceCode/).
* If you haven't already, please git clone this repo to your local machinne
	* ```git clone https://github.com/PlayFab/gsdk.git```
	* Later steps will involve integrating files from this repo to your project
* [Optional] Download the [LocalMultiplayerAgent](https://github.com/PlayFab/MpsAgent/releases)
	* [Optional] Alternatively, download [LocalMultiplayerAgent source](https://github.com/PlayFab/MpsAgent/tree/main/LocalMultiplayerAgent).
* [Optional] [PlayFab Marketplace plugin](https://www.unrealengine.com/marketplace/product/playfab-sdk) or the [source version on GitHub](https://github.com/PlayFab/UnrealMarketplacePlugin/tree/master/4.26/PlayFabPlugin/PlayFab). This plugin is not required for GSDK but is required for many PlayFab services, including login.

## Project creation

The Unreal server project needs to be a network-enabled multiplayer Unreal project with a dedicated-server mode. If you don't have a project that meets these prerequisites, follow our [Unreal prerequisite set up guide](ThirdPersonMPSetup.md) to set one up. Once you have a network-enabled, multiplayer game, with a dedicated server, return to this step and continue.

When ready, open your network-enabled multiplayer Unreal server project, and continue to the next step to install the Unreal GSDK.

## Project GSDK setup

* GSDK Project Prerequisite details
	* Dedicated Server:
		* Your project should have a separate {ProjectName}Server.Target.cs file
		* This file should contain ```Type = TargetType.Server;``` to enable dedicated server mode
		* When built in "development server" configuration for Win64, your output contains {ProjectName}Server.exe
	* Network-enabled:
		* There are multiple ways to meet this requirement, but the simple answer is: your client can establish a network connection with a game server on a separate machine, and communicate meaningfully
		* Our instructions direct you to do this through Unreal's built-in data replication system which handles most of this for you
		* A more direct way to do this is with direct socket use, such as through the ISocketSubsystem, which is a much more advanced topic
	* Multiplayer:
		* Your game server can accept multiple incoming client connections, and communicate properly with both
		* Again, with Unreal's built-in data replication, this is mostly handled for you

With a properly configure multiplayer project, you're ready to [install the GSDK](ThirdPersonMPGSDKSetup.md). This will allow your server to run on PlayFab Multiplayer Services.

## Unreal Project Build Configurations

Once the GSDK is installed and configured in your project, you can [build your Unreal project](ThirdPersonMPBuild.md). There are multiple build configurations you will need to execute the full range of tests in future steps.

## Test Local Server Deployment

Once you have any client build, and a "Development Server" build, you can [test the MPS deployment process locally](ThirdPersonMPLocalDeploy.md), and verify your server works.

## Deploy to PlayFab

Finally, you're ready to [create game servers in the cloud](ThirdPersonMPCloudDeploy.md) using PlayFab Multiplayer Services.

# Acknowledgements

We would like to express our gratitude to the following people and projects for their contributions to this project:

- [Stefan Krismann](https://github.com/stkrwork) for the excellent work implementing this plugin and contributing it to the community.
- [Nick Arthur](https://github.com/narthur157) for his amazing [Dockerfile](https://github.com/narthur157/playfab-gsdk-ue4).
- [Adam Rehn](https://github.com/adamrehn) for all his work with Unreal and Docker, including code, samples, examples, and tutorials.
