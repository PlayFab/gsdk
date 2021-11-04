# ThirdPersonMP Example Project Local Deployment and Debugging

The purpose of this guide is to demonstrate running your game-server on your local machine, in a MPS-compliant way, so you can test and debug your server before uploading it to PlayFab.

## Goals

* Test the local deployment options for the ThirdPersonMP+GSDK project.
* Verify your server executes properly when run using LocalMultiplayerAgent
* Verify you can attach a debugger to your server instance

## Requirements

* Download Visual Studio (the [community version is free](https://visualstudio.microsoft.com/vs/community/))
	- Requires Workloads: .NET desktop development, and Desktop development with C++
* Download Unreal Engine Source Build by following [these instructions](https://docs.unrealengine.com/4.26/en-US/ProgrammingAndScripting/ProgrammingWithCPP/DownloadingSourceCode/) from the Unreal Engine website. This was tested on [4.26.2](https://github.com/EpicGames/UnrealEngine/releases/tag/4.26.2-release) (Link requires permissions)
* [Completed Unreal Project](ThirdPersonMPSetup.md) with [PlayFab Unreal GSDK](ThirdPersonMPGSDKSetup.md) installed and configured
* ["Development Server"](ThirdPersonMPBuild.md) configuration of your project built from Visual Studio
* Download the [LocalMultiplayerAgent](https://github.com/PlayFab/MpsAgent/tree/main/LocalMultiplayerAgent)
* "Debug" or "Release" configuration of LocalMultiplayerAgent built from this repo with Visual Studio
	* Load [MpsAgent.sln](https://github.com/PlayFab/MpsAgent/blob/main/MpsAgent.sln) in Visual Studio, select configuration, build LocalMultiplayerAgent project
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
  "ProcessStartParameters": {
    "StartGameCommand": "<PATH TO EXE> -log"
  },
  "ContainerStartParameters": {
    "StartGameCommand": "C:\\Assets\\ThirdPersonMPServer.exe -log",
	...
  }
}
```

The parts of the file obscured by ```...``` above, can be ignored in this guide. The purpose and values for the important fields are as follows:

* RunContainer: For this guide, this will always be false. Setting this to true requires Docker.
    * When true, the zip file in <PATH TO ZIP> must contain the entire project server release build
	* When true, the ProcessStartParameters/StartGameCommand is ignored, and ContainerStartParameters/StartGameCommand is used instead
	* When true, everything is built and run in a docker container, rather than on the local machine
	* This guide covers the scenario when RunContainer is false, so that we can more easily debug the server process
* AssetDetails/LocalFilePath: <PATH TO ZIP>
	* This location must be fully defined, and a valid zip file must exist at this location
	* When RunContainer==false, this file is ignored, but its existence is still required
	* The zip file can be empty
	* This requirement is a low priority polish item that hasn't been reconsidered yet
	* For this example, this could be: ```{depot}\\empty.zip```
	* For the author, this is: ```"M:\\depot\\GSDK\\ThirdPersonMPGSDK\\Binaries\\Win64\\ThirdPersonMPServer.zip```, with an empty zip file at that location
	* NOTE: When RunContainer==true, the contents of this file will be relevant and used
* ContainerStartParameters/StartGameCommand: While RunContainer is false, this is unused.
	* When RunContainer==true, it supercedes the ProcessStartParameters/StartGameCommand
	* This path is the internal path within the docker container, and will be the sum of AssetDetails/MountPath, plus the internal path-to-exe in your zip file
	* For this example, this could be: ```C:\\Assets\\ThirdPersonMPServer.exe -log```
* ProcessStartParameters/StartGameCommand: This command will effectively be the path to your exe, and any command-line parameters used to start your game server
	* -log is an Unreal command to instruct the game-server to save an execution log
	* <PATH TO EXE> can be any local path to the exe for your game server, plus any command line parameters for your server
	* For this example, this could be: ```{depot}\\ThirdPersonMP\\Binaries\\Win64\\ThirdPersonMPServer.exe -log```
	* For the author, this is: ```M:\\depot\\GSDK\\ThirdPersonMPGSDK\\Binaries\\Win64\\ThirdPersonMPServer.exe -log```

Once you have created your zip file and set all of these lines to appropriate values, you can rebuild your LocalMultiplayerAgent, and prepare to debug your server.

### Debugging your server

You can run LocalMultiplayerAgent from Visual Studio with the "Start New Instance" command, sometimes bound to F5, or you can navigate to ```{depot}\LocalMultiplayerAgent\bin\{configuration}\netcoreapp3.1``` and double click "LocalMultiplayerAgent.exe". You can also run this from within a cmd window to observe or capture debug log information.

Running LocalMultiplayerAgent.exe should start your game server. You can find your game server process ID in the Details column. If there are multiple instances, you may need to use Task 
Manager to force-close instances that are lingering from previous attempts. If your LocalMultiplayerAgent _starts_ multiple instances, look for the process ID with a comparatively high memory use (Some Unreal build configurations have multiple executables, and both are in the task manager for the duration).

Once you see your ThirdPersonMPServer process running in Task Manager, you can return to Visual Studio, select the Debug dropdown -> Attach to Process. From the popup window, you can search your process name: ThirdPersonMPServer, and then select the proper process ID, identified from Task Manager.

At this point, you should be able to perform moderate debugging into your game.

NOTE: Unreal provides multiple build configurations and multiple ways to build your server. For best results, use the "Development Server" configuration, and output, built directly from Visual Studio. Release builds, or builds built from the Development Editor may work better for other situations, but the configuration built directly from Visual Studio will be easier to attach, and debug in Visual Studio.

## Navigation

You are now ready to deploy [your server to the cloud](ThirdPersonMPCloudDeploy.md).

Alternately, you can return to the main [Unreal GSDK Plugin](README.md#deploy-to-playfab) guide.
