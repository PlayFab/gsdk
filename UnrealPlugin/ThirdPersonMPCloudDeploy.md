# ThirdPersonMP Example Project Cloud Deployment

This guide explains how to deploy your game to PlayFab Multiplayer Server (MPS) cloud. This is the final step in the entire game server cloud deployment process.

## Goals

* Deploy a game to the cloud.
* Connect to it from a local game client

## Requirements

* Download Visual Studio. The [community version](https://visualstudio.microsoft.com/vs/community/) is free.
	* Required workloads: .NET desktop development and Desktop development with C++
* Download Unreal Engine Source Code. This plugin was tested on Unreal Engine 4.26.2. For instructions, see [Downloading Unreal Engine Source code (external)](https://docs.unrealengine.com/4.26/en-US/ProgrammingAndScripting/ProgrammingWithCPP/DownloadingSourceCode/).
* [Completed Unreal Project](ThirdPersonMPSetup.md) with [PlayFab Unreal GSDK](ThirdPersonMPGSDKSetup.md) installed and configured
* [Release Server](ThirdPersonMPBuild.md) configuration of your project built from Visual Studio or Development Editor
* [Client](ThirdPersonMPBuild.md) configuration of your project built from Visual Studio or Development Editor

## Instructions

### Packaging the game server

Make sure that you have just built your project in development editor in Visual Studio *before* doing this next step, because sometimes building fails when opening the project directly through the .uproject file.

Right click on the .uproject file in your file editor and select "Generate Visual Studio Project Files".

Then open the .uproject file.

In the top left of the Unreal editor menu, click on File -> Package -> Target Configuration -> [game name]Server and then go to File -> Package -> Windowsx64.
You can now use this packaged version of your game server to [test with LocalMultiplayerAgent](https://docs.microsoft.com/gaming/playfab/features/multiplayer/servers/locally-debugging-game-servers-and-integration-with-playfab)
or to use it directly with PlayFab by [creating a build](https://developer.playfab.com/) and then using [PlayFab MpsAllocatorSample](https://github.com/PlayFab/MpsSamples/blob/master/MpsAllocatorSample/README.md).

### Setting up a Windows Dedicated Server on PlayFab

An important note about this is that you need to set the start command in process-based mode to \<root folder\>\\Binaries\\Win64\\\<project name\>Server.exe (or however the executable is called in the folder).

In container-based mode use \<mount folder\>\\\<root folder\>\\Binaries\\Win64\\\<project name\>Server.exe (or however the executable is called in the folder).

**If you use the executable in the root folder, the server will fail to initialize with the PlayFab system.**

### Setting up a Linux Dedicated Server on PlayFab

During testing the following Dockerfile + startup.sh script worked excellent with the PlayFab Linux VM:

#### The Linux Dockerfile:

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

### startup.sh bash script:

```bash
chown -R ue.ue $PF_SERVER_LOG_DIRECTORY
su ue -c ./<projectname>Server.sh
```
Make sure that the line endings in the startup.sh file are LF (\\n) and not CRLF (\\r\\n).

# Navigation

This guide sequence is finished. You can return to the main [Unreal GSDK Plugin](README.md) guide.
