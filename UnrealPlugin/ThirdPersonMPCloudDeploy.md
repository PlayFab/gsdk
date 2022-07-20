# ThirdPersonMP Example Project Cloud Deployment

This guide explains how to deploy your game to PlayFab Multiplayer Server (MPS) cloud. This is the final step in the entire game server cloud deployment process.

## Goals

* Deploy a game to the cloud.
* Connect to it from a local game client

## Requirements

* Download Visual Studio. The [community version](https://visualstudio.microsoft.com/vs/community/) is free.
	* Required workloads: .NET desktop development and Desktop development with C++
* Download Unreal Engine Source Code. This plugin was tested on Unreal Engine 4.26.2. For instructions, see [Downloading Unreal Engine Source code (external)](https://docs.unrealengine.com/4.26/ProgrammingAndScripting/ProgrammingWithCPP/DownloadingSourceCode/).
* [Completed Unreal Project](ThirdPersonMPSetup.md) with [PlayFab Unreal GSDK](ThirdPersonMPGSDKSetup.md) installed and configured
* [Release Server](ThirdPersonMPBuild.md) configuration of your project built from Visual Studio or Development Editor
* [Client](ThirdPersonMPBuild.md) configuration of your project built from Visual Studio or Development Editor
* MPS is [enabled for your title](https://docs.microsoft.com/gaming/playfab/features/multiplayer/servers/enable-playfab-multiplayer-servers), and billing is set up

## Instructions

### Packaging the game server

* [OPTIONAL] This section can be done to refresh the files which Unreal builds into your project. This is useful if you've recently created or deleted files.
	* If you have not recently built your project in "development editor" configuration, do so now
	* Close Visual Studio
	* Navigate to the .uproject file for your project
	* Right-click on the .uproject file in your file editor and select "Generate Visual Studio Project Files"	
	* Re-open the {ProjectName}.sln file in Visual Studio
* Select __Development Editor__ configuration
* Run your project
* Observe the Unreal Editor, which should have loaded your project
* In the top left of the Unreal editor menu, Select __File__ -> __Package__ -> __Build Target__ -> __{ProjectName}Server__
* Select __File__ -> __Package__ -> __Build Configuration__ -> __Shipping__
* Select __File__ -> __Package__ -> __Windowsx64__
* Select and remember an output location {ShippingServerFolder}
* You can now use this packaged version of your game server to [test with LocalMultiplayerAgent](https://docs.microsoft.com/gaming/playfab/features/multiplayer/servers/locally-debugging-game-servers-and-integration-with-playfab)
* Once verified with LocalMultiplayerAgent, see [creating a build guide](https://docs.microsoft.com/gaming/playfab/features/multiplayer/servers/deploying-playfab-multiplayer-server-builds)
	* Testing on MPS at this point will require an intermediate tool from [PlayFab MpsAllocatorSample](https://github.com/PlayFab/MpsSamples/blob/main/MpsAllocatorSample/README.md)
	* This tool bypasses some client work for now, and allows us to allocate a server without a direct request from the game client
	* The proper way to do this on a client is a separate topic covered in the [allocation guide](https://docs.microsoft.com/gaming/playfab/features/multiplayer/servers/allocating-game-servers-and-configuring-vs-debugging-tools).
* Zip your Shipping Server build from {ShippingServerFolder} above
	* Your zip should contain everything in {ShippingServerFolder}, optionally at the root of the zip file
	* [Optional] IE, the root of your zip should contain {ProjectName}Server.exe (It's suggested the exe should not be in a subfolder such as "WindowsServer". Rebuild your zip if desired)
* Get familiar with the [build creation process guide](https://docs.microsoft.com/gaming/playfab/features/multiplayer/servers/deploying-playfab-multiplayer-server-builds)
* Specific details for specific configurations are detailed below:

### Setting up a Windows Dedicated Server on PlayFab

This section will cover creating a build in PlayFab MPS, uploading your game server, and connecting to it for the first time.

* Navigate to Game Manager -> Multiplayer
* Select __New Build__
* Give the build a name
* Select a Virtual Machine (For the samples described in these guides, the recommended option with the fewest cores is sufficient)
* Servers per machine can be aproximately 1~10
	* Improving/Raising this number is based on the performance of your game server
* Skip __Server details__ for now (covered in the next step)
* In the Assets block, drag your zip file from the previous section into the upload box
	* If you're re-launching, you can select __Use existing assets__
* Wait for the upload to complete
* Skip __Start Command__ for now (covered in the next step)
* In the Network block, set the Port Name field to ```gameport```
	* It's mandatory to define the port that will be used to connect to your game server
	* Unreal GSDK contains code to read this port automatically, and internaly override the Unreal Server host port according to this value
	* It requires that the name of this port be set properly to ```gameport```
* Continue to the next step that matches the "Server Details" that you want to configure

#### Option 1: Windows Process Mode

This option runs your game server(s) as a process directly on the VM. Your processes will share access to the full machine, including ports, drives, etc. This option unzips your asset upload, and executes the "Start Command" from the location of your unzipped files as the working directory.

* In __Server Details__, select __Process__ and __Windows__
* In __Assets__, set the start command to:
	* ```{ProjectName}Server.exe``` or ```{ProjectName}Server.exe -log``` if you want to generate log files
	* The start command is executed from the command line, using your unzipped asset files as the working directory
	* If you chose to pack your zip file with sub-folders, you can specify a relative path such as: ```WindowsServer/{ProjectName}Server.exe``` to access your project with a relative sub-path
* Select __Add Build__
* Skip ahead to the __Configure Regions__ section below

#### Option 2: Windows Container Mode

This option runs your game server(s) in docker containers, isolating them from eachother. Each server process will have a separate virtual drive, virtual network ports, etc. This option unzips your asset to a Mount Path you will define, and expects your start command to be a full path, based on the Mount Path.

* In __Server Details__, select __Process__ and __Windows__
* In __Assets__
	* Set the __Mount Path__ for your gameServer zip file to ```C:\assets``` (or something similar, denoted as {MountPath})
	* Set the start command to: ```{MountPath}\{ProjectName}Server.exe``` or ```{MountPath}\{ProjectName}Server.exe -log``` if you want to generate log files
	* The start command should represent a full path in the container, to your -Server.exe file
	* If you chose to pack your zip file with sub-folders, you can specify an absolute path such as: ```{MountPath}\WindowsServer\{ProjectName}Server.exe``` to access your project with an absolute sub-path
* Select __Add Build__
* Skip ahead to the __Configure Regions__ section below

### Setting up a Linux Dedicated Server on PlayFab

A more detailed Linux guide can be found [here](https://github.com/PlayFab/MpsAgent/blob/main/LocalMultiplayerAgent/MultiplayerSettingsLinuxContainersOnWindowsSample.json)

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

#### Configure Regions

A new build is configured without regions, and will idle in an unfinished state. By adding regions, we'll launch actual servers, and be ready to connect.

* Select the __New Build__ button. This will take you to the __Regions__ page of the newly created build.
* Select __Add Region__
	* In the prerequisites, you will have reserved some availability in some predefined regions.
	* Select an available region close to you
	* Target Standby: 1
	* Maximum: 1
* Submit
* Wait aproximately 10 minutes for a server to launch

### Connect Game Client to MPS hosted build

(Under construction, coming soon!)

# Navigation

This guide sequence is finished. You can return to the main [Unreal GSDK Plugin](README.md) guide.
