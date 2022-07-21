# ThirdPersonMP Example Project Local Deployment and Debugging

The purpose of this guide is to demonstrate running your game-server on your local machine, in a MPS-compliant way, so you can test and debug your server before uploading it to PlayFab.

## Goals

* Test the local deployment options for the ThirdPersonMP+GSDK project.
* Verify your server executes properly when run using LocalMultiplayerAgent
* Verify you can attach a debugger to your server instance

## Requirements

* Download Visual Studio. The [community version](https://visualstudio.microsoft.com/vs/community/) is free.
	* Required workloads: .NET desktop development and Desktop development with C++
* Download Unreal Engine Source Code. This plugin was tested on Unreal Engine 4.26.2. For instructions, see [Downloading Unreal Engine Source code (external)](https://docs.unrealengine.com/4.26/ProgrammingAndScripting/ProgrammingWithCPP/DownloadingSourceCode/).
* [Completed Unreal Project](ThirdPersonMPSetup.md) with [PlayFab Unreal GSDK](ThirdPersonMPGSDKSetup.md) installed and configured
* ["Development Server"](ThirdPersonMPBuild.md) configuration of your project built from Visual Studio
* [Optional] Download the [LocalMultiplayerAgent](https://github.com/PlayFab/MpsAgent/releases)
	* [Optional] Alternatively, download [LocalMultiplayerAgent source](https://github.com/PlayFab/MpsAgent/tree/main/LocalMultiplayerAgent).
* "Debug" or "Release" configuration of LocalMultiplayerAgent built from this repo with Visual Studio
	* Using Visual Studio, Open [MpsAgent.sln](https://github.com/PlayFab/MpsAgent/blob/main/MpsAgent.sln), Select either Debug or Release configuation, and then build the LocalMultiplayerAgent.
* [Optional] Install [Docker for Windows](https://www.docker.com/products/docker-desktop)

## Notation

{depot} will refer to the full windows path for the location where you download your Git projects. These can be anywhere you like, such as: C:\depot, S:\depot, Z:\gitrepos or whichever drive and path is convienient for you. It is typically recommended (especially for Unreal), that your {depot} path be very short, if possible. For the author, {depot} resolves to: ```M:\depot\GSDK```. For example, with the requirements list above, you will likely have some or all of the following:

* {depot}/ThirdPersonMP
* {depot}/MpsAgent
* [Optional] {depot}/gsdk [This contains the PlayFab Unreal GSDK plugin previously installed into ThirdPersonMP]
* [Optional] {depot}/UnrealMarketplacePlugin  [This contains the PlayFab Unreal Marketplace plugin, which is not required for this guide, but almost certainly required for other PlayFab features]

It is not required that you have all of these in the same location, but it is likely useful to do so, and for organization, this guide encourages you to do so.

## Instructions

### Local execution, no containers

First, you will need to configure your LocalMultiplayerAgent to execute your server project. The first iteration will run the process directly on your local PC without any isolation.

In Explorer, find and open the file: ```{depot}\MpsAgent\LocalMultiplayerAgent\MultiplayerSettings.json```. An abbreviated version of this file with the parts important to this guide are as follows [NOTE the escaped \\'s for paths in the json - this is a json file, and thus it's required to escape all \\'s as \\\\]:

```json
{
  "RunContainer": false,
  ...
  "AssetDetails": [
    {
      "MountPath": "C:\\Assets",
      "LocalFilePath": "<PATH TO ZIP>"
    }
  ],
  ...
  "PortMappingsList": [
    [
      {
        "NodePort": 30000,
        "GamePort": {
          "Name": "gameport",
          "Number": 8888,
          "Protocol": "UDP"
        }
      }
    ]
  ],
  "ProcessStartParameters": {
    "StartGameCommand": "<PATH TO EXE> -log"
  },
  "ContainerStartParameters": {
    "StartGameCommand": "C:\\Assets\\ThirdPersonMPServer.exe -log",
	...
  }
}
```

For the purposes of this guide, the parts of the json file obscured by ```...``` above, just utilize the project defaults. The purpose and values for the important fields are as follows:

* RunContainer: For this guide, this will always be false. Setting this to true requires Docker.
	* When true, the ProcessStartParameters/StartGameCommand is ignored, and ContainerStartParameters/StartGameCommand is used instead
	* When true, everything is built and run in a docker container, rather than on the local machine context
	* This guide covers the scenario when RunContainer is false, so that we can more easily debug the server process
	* Setting this to true requires a Shipping Server build, additional [Windows](https://docs.adamrehn.com/ue4-docker/configuration/configuring-windows-server) or [Linux](https://unrealcontainers.com/blog/identifying-application-runtime-dependencies/) DLLs, and [Docker for Windows](https://www.docker.com/products/docker-desktop)
* AssetDetails/LocalFilePath: <PATH TO ZIP>
	* This location must be fully defined, and a valid zip file must exist at this location
	* This zip file should contain a fully constructed "Shipping Server" build
* PortMappingsList:
	* This is the LocalMultiplayerAgent equivalent of defining the port in Game Manager
	* The GSDK is hard-coded to look for a port with the name: gameport
	* The GSDK [SetDefaultServerHostPort](https://github.com/PlayFab/gsdk/blob/master/UnrealPlugin/Source/PlayfabGSDK/Private/GSDKUtils.cpp#L147) method will internally override the internal Unreal Game Server hosting port to match this port
	* This is because MPS must be the source of which port a game server will use, due to the way MPS launches servers, particularly when running multiple on the same cloud VM
	* This json let's you locally test driving this port number from MPS, and ensures your GSDK plugin will receive it
* ContainerStartParameters/StartGameCommand: While RunContainer is false, this is unused.
	* When RunContainer==true, it supercedes the ProcessStartParameters/StartGameCommand
	* This path is the internal path within the docker container, and will be the sum of AssetDetails/MountPath, plus the internal path-to-exe in your zip file defined by AssetDetails/LocalFilePath
	* For this example, this could be: ```C:\\Assets\\ThirdPersonMPServer.exe -log```
* ProcessStartParameters/StartGameCommand: This command will effectively be the path to your exe, and any command-line parameters used to start your game server
	* -log is an Unreal command to instruct the game-server to save an execution log
	* <PATH TO EXE> can be one of two choices:
		* Any absolute path to an exe for your game server (even development game server), plus any command line parameters for your server
			* This choice will ignore the contents of the zip file, and instead execute any arbitrary exe in any location
			* This is a local debug option that only works on your local machine to debug development builds: It does not help you verify your zip file is ready to upload to MPS
			* This choice should be used when testing a development server, suitable for attaching a VS debugger
			* For this example, this could be: ```{depot}\\ThirdPersonMP\\Binaries\\Win64\\ThirdPersonMPServer.exe -log```
			* For the author, this is: ```M:\\depot\\GSDK\\ThirdPersonMPGSDK\\Binaries\\Win64\\ThirdPersonMPServer.exe -log```
		* A relative path, which should indicate the relative path into your zip file, to run your server
			* This is the standard workflow that mirrors how it works on a MPS cloud instance
			* This choice should be used when testing a shipping server, and helps verify your zip is ready to upload to MPS
			* For this example, this could be: ```ThirdPersonMPServer.exe -log```

Once you have created your zip file and set all of these lines to appropriate values, you can rebuild your LocalMultiplayerAgent, and prepare to debug your server.

### Debugging your server

You can run LocalMultiplayerAgent from Visual Studio with the "Start New Instance" command, sometimes bound to F5, or you can navigate to ```{depot}\LocalMultiplayerAgent\bin\{configuration}\netcoreapp3.1``` and double-click "LocalMultiplayerAgent.exe". You can also run this from within a cmd window to observe or capture debug log information.

Running LocalMultiplayerAgent.exe should start your game server. You will usually want to have Task Manager open for this. You can find your game server process ID in the Details tab of Task Manager.

Once you see your ThirdPersonMPServer process running in Task Manager, you can return to Visual Studio, select the Debug dropdown -> Attach to Process. From the popup window, you can search your process name: ThirdPersonMPServer, and then select the proper process ID, identified from Task Manager.

At this point, you should be able to perform typical debugging into your game server.

NOTE: Unreal provides multiple build configurations and multiple ways to build your server. For best results, use the "Development Server" configuration, and output, built directly from Visual Studio. Shipping builds, or builds built from the Development Editor may work better for other situations, but the "Development Server" configuration built directly from Visual Studio will be easier to attach and debug in Visual Studio.

## Troubleshooting

### Many instances of ThirdPersonMPServer in Task Manager

If there are multiple instances listed (usually 3 or more), you may need to use Task Manager to force-close instances that are lingering from previous attempts.

### LocalMultiplayerAgent launches 2 instances of ThirdPersonMPServer

If your LocalMultiplayerAgent _starts_ multiple instances, look for the process ID with a comparatively high memory use. Some configurations of the game server generate two executables, which run as a pair. You will want to attach to the one with a much higher memory use.

## Navigation

You are now ready to deploy [your server to the cloud](ThirdPersonMPCloudDeploy.md).

Alternately, you can return to the main [Unreal GSDK Plugin](README.md#deploy-to-playfab) guide.
