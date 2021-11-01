# ThirdPersonMP Example Project Setup

This guide coveres the construction of an example project which can operate as a game-client and game-server, and is configured to operate on PlayFab Multiplayer Services.

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

All of the necessary instructions are in the Unreal Tutorials. First, you should download/configure the 4.26.2 Unreal Engine source. Then, you should proceed to the Unreal [Multiplayer Programming Quick Start](https://docs.unrealengine.com/4.27/en-US/InteractiveExperiences/Networking/QuickStart/) tutorial.

Once finished with the above guide, you will have a working multiplayer game client. Next, we need a dedicated server. At this point, you should proceed to the Unreal [Setting Up Dedicated Servers](https://docs.unrealengine.com/4.27/en-US/InteractiveExperiences/Networking/HowTo/DedicatedServers/) tutorial.

Once finished with both guides, you will have a network-enabled multiplayer game with a dedicated game server.

You can now proceed to install PlayFab MPS using the [Unreal GSDK Plugin](UnrealPlugin/README.md) guide.
