# ThirdPersonMP Example Project Setup

This guide coveres the construction of an example project which can operate as a game-client and game-server. This will set up the minimum requirements for an Unreal game that can be hosted on PlayFab Multiplayer Services.

## Goals

The project needs to have the following capabilities and features:

* Networking
* Multiplayer
* Dedicated Game-Server
* PlayFab MPS Support -- ??? NEXT GUIDE ?!?

To accomplish this, we will construct a project from scratch using Unreal tutorials.

## Requirements

* Visual Studio Community 2019
* Unreal 4.26.2 Engine Source

## Instructions

All of the necessary instructions are in the Unreal Tutorials. First, download Unreal Engine Source Build by following [these instructions](https://docs.unrealengine.com/4.26/en-US/ProgrammingAndScripting/ProgrammingWithCPP/DownloadingSourceCode/) from the Unreal Engine website. This was tested on [4.26.2](https://github.com/EpicGames/UnrealEngine/releases/tag/4.26.2-release) (Link requires permissions).

Next, you should follow the Unreal [Third Person Template](https://docs.unrealengine.com/4.27/en-US/Resources/Templates/ThirdPerson/) tutorial. This creates the basic project we will use for the rest of this document.

Then, you should proceed to the Unreal [Multiplayer Programming Quick Start](https://docs.unrealengine.com/4.27/en-US/InteractiveExperiences/Networking/QuickStart/) tutorial. NOTE, the first step of this tutorial is an abbreviated version of the one above. It upgrades the ThirdPerson project to a multiplayer project.

Once finished with the above guides, you will have a working multiplayer game client. Next, we need a dedicated server. At this point, you should proceed to the Unreal [Setting Up Dedicated Servers](https://docs.unrealengine.com/4.27/en-US/InteractiveExperiences/Networking/HowTo/DedicatedServers/) tutorial.

Once finished with all guides, you will have a network-enabled multiplayer game with a dedicated game server.

You can now return to the [Unreal GSDK Plugin](README.md#project-setup) guide and continue your PlayFab MPS setup by installing the PlayFab Unreal GSDK.
