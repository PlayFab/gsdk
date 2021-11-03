# Instructions on Integrating the PlayFab GSDK Unreal Plugin Preview

This Unreal Plugin is implementing the GSDK directly in Unreal Engine.

This plugin offers both a Blueprint API and a C++ API. This does however not mean that the Blueprint API does not require the game to be a C++ project. If it is currently a Blueprint only project, then you need to convert it beforehand, before adding the plugin.

The following nodes are supported:
![PlayFab GSDK Blueprint Nodes](Documentation/GSDKBlueprintNodes.png)

It was tested with Unreal Engine 4.26.2, but should work with other engines as well.

The sample game these instructions were created with was called ThirdPersonMP, so replace anywhere you see that with your game name.

# Prerequisites:

- Download Visual Studio (the [community version is free](https://visualstudio.microsoft.com/vs/community/))
	- Requires Workloads: .NET desktop development, and Desktop development with C++
- Download Unreal Engine Source Build by following [these instructions](https://docs.unrealengine.com/4.26/en-US/ProgrammingAndScripting/ProgrammingWithCPP/DownloadingSourceCode/) from the Unreal Engine website. This was tested on [4.26.2](https://github.com/EpicGames/UnrealEngine/releases/tag/4.26.2-release) (Link requires permissions)
- Download the [Unreal PlayFabGSDK Plugin folder](https://github.com/PlayFab/gsdk/tree/master/UnrealPlugin)
- [Optional] Download the [LocalMultiplayerAgent](https://github.com/PlayFab/MpsAgent/tree/main/LocalMultiplayerAgent)

# Project creation

Unreal GSDK is installed into an Unreal server project. You will need a network-enabled multiplayer Unreal project with a dedicated-server mode. If you do not have one, you can follow our [Unreal prerequsitie setup](ThirdPersonMPSetup.md) guide. Once you have a network-enabled, multiplayer game, with a dedicated server, return here and continue.

# Project GSDK setup

With a multiplayer project configured for both a network connected game-client and game-server, you're ready to [install the GSDK](ThirdPersonMPGSDKSetup.md). This will allow your server to run on PlayFab Multiplayer Services.

# Unreal Project Build Configurations

Once the GSDK is installed and configured in your project, you can [build your Unreal project](ThirdPersonMPBuild.md). There are multiple build configurations you will need to execute the full range of tests in future steps.

# Test Local Server Deployment

Once you have any client build, and a "Development Server" build, you can [test the MPS deployment process locally](ThirdPersonMPLocalDeploy.md), and verify your server works.

# Deploy to PlayFab

Finally, you're ready to deploy your game-server to PlayFab, and test with real game servers in the cloud. _TODO-LINK!_





TODO: MOVE THIS TO PLAYFAB-DEPLOY FILE

## Packaging the game server

Make sure that you have just built your project in development editor in Visual Studio *before* doing this next step, because sometimes building fails when opening the project directly through the .uproject file.

Right click on the .uproject file in your file editor and select "Generate Visual Studio Project Files". 

Then open the .uproject file.

In the top left of the Unreal editor menu, click on File -> Package -> Target Configuration -> [game name]Server and then go to File -> Package -> Windowsx64. 
You can now use this packaged version of your game server to [test with LocalMultiplayerAgent](https://docs.microsoft.com/en-us/gaming/playfab/features/multiplayer/servers/locally-debugging-game-servers-and-integration-with-playfab) 
or to use it directly with PlayFab by [creating a build](https://developer.playfab.com/) and then using [PlayFab MpsAllocatorSample](https://github.com/PlayFab/MpsSamples/blob/master/MpsAllocatorSample/README.md).

## Setting up a Windows Dedicated Server on PlayFab

An important note about this is that you need to set the start command in process-based mode to \<root folder\>\\Binaries\\Win64\\\<project name\>Server.exe (or however the executable is called in the folder).

In container-based mode use \<mount folder\>\\\<root folder\>\\Binaries\\Win64\\\<project name\>Server.exe (or however the executable is called in the folder).

**If you use the executable in the root folder, the server will fail to initialize with the PlayFab system.**

## Setting up a Linux Dedicated Server on PlayFab

During testing the following Dockerfile + startup.sh script worked excellent with the PlayFab Linux VM:

### The Dockerfile:

```Dockerfile
FROM ubuntu:18.04

# Unreal refuses to run as root user, so we must create a user to run as
# Docker uses root by default
RUN useradd --system ue
USER ue

EXPOSE 7777/udp

WORKDIR /server

COPY --chown=ue:ue . /server
USER root
CMD ./startup.sh
```

### startup&#46;sh bash script:

```bash
chown -R ue.ue $PF_SERVER_LOG_DIRECTORY
su ue -c ./<projectname>Server.sh
```
Make sure that the line endings in the startup&#46;sh file are LF (\\n) and not CRLF (\\r\\n).

Thank you to [narthur157](https://github.com/narthur157) for his amazing [Dockerfile](https://github.com/narthur157/playfab-gsdk-ue4), which this Dockerfile is based on.
