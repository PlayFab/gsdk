# ThirdPersonMP Example Project Setup

This guide covers the construction of an example project which can operate as a game-client and dedicated game-server. This will set up the minimum requirements for an Unreal project, before the GSDK can be integrated.

This guide is an excellent starting point if you are starting from scratch.

## Goals

The project needs to have the following capabilities and features:

* Networking
* Multiplayer
* Dedicated Game-Server

To accomplish this, we will construct a project from scratch using Unreal tutorials.

## Requirements

- Download Visual Studio (the [community version is free](https://visualstudio.microsoft.com/vs/community/))
	- Requires Workloads: .NET desktop development, and Desktop development with C++
- Download Unreal Engine Source Build by following [these instructions](https://docs.unrealengine.com/4.26/en-US/ProgrammingAndScripting/ProgrammingWithCPP/DownloadingSourceCode/) from the Unreal Engine website. This was tested on [4.26.2](https://github.com/EpicGames/UnrealEngine/releases/tag/4.26.2-release) (Link requires permissions)

## Instructions

All of the necessary instructions are in the Unreal Tutorials. First, download Unreal Engine Source Build by following [these instructions](https://docs.unrealengine.com/4.26/en-US/ProgrammingAndScripting/ProgrammingWithCPP/DownloadingSourceCode/) from the Unreal Engine website. This was tested on [4.26.2](https://github.com/EpicGames/UnrealEngine/releases/tag/4.26.2-release) (Link requires permissions).

Next, you should follow the Unreal [Third Person Template](https://docs.unrealengine.com/4.27/en-US/Resources/Templates/ThirdPerson/) tutorial. This covers the creation of a basic project we will use for the rest of this document.

Then, you should proceed to the Unreal [Multiplayer Programming Quick Start](https://docs.unrealengine.com/4.27/en-US/InteractiveExperiences/Networking/QuickStart/) tutorial. Note, the first step of this tutorial is an abbreviated version of the one above. This guide upgrades the ThirdPersonMP template to a multiplayer project.

Once finished with the above guides, you will have a working multiplayer game client. Next, you need a dedicated server. At this point, you should proceed to the Unreal [Setting Up Dedicated Servers](https://docs.unrealengine.com/4.27/en-US/InteractiveExperiences/Networking/HowTo/DedicatedServers/) tutorial. This will set up a dedicated server for your project, and concludes our list of requirements.

Once finished with all guides, you will have a network-enabled multiplayer project with a dedicated game server.

## Navigation

You are now ready to [install the PlayFab Unreal GSDK](ThirdPersonMPGSDKSetup.md) into this project.

Alternately, you can return to the main [Unreal GSDK Plugin](README.md#project-gsdk-setup) guide.