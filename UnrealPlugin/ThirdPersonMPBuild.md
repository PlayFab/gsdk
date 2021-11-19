# Building the ThirdPersonMP Example Project

The purpose of this guide is to describe all the build configurations needed to test and verify your Unreal project.

## Goals

* Build the "Development Editor"
* Build a game client
* Build the "Development Server"
* Build a release Server

## Requirements

* Download Visual Studio. The [community version](https://visualstudio.microsoft.com/vs/community/) is free.
	* Required workloads: .NET desktop development and Desktop development with C++
* Download Unreal Engine Source Code. This plugin was tested on Unreal Engine 4.26.2. For instructions, see [Downloading Unreal Engine Source code (external)](https://docs.unrealengine.com/4.26/ProgrammingAndScripting/ProgrammingWithCPP/DownloadingSourceCode/).
* [Completed Unreal Project](ThirdPersonMPSetup.md) with [PlayFab Unreal GSDK](ThirdPersonMPGSDKSetup.md) installed and configured
* Knowledge about [Unreal Build Configuration](https://docs.unrealengine.com/4.27/ProductionPipelines/DevelopmentSetup/CompilingProjects/) options

## Instructions

The [GSDK setup](ThirdPersonMPGSDKSetup.md) guide previously described the steps to "Switch Unreal Engine Version" and "Generate Visual Studio project files". There is also an [Unreal guide](https://docs.unrealengine.com/4.27/ProductionPipelines/DevelopmentSetup/CompilingProjects/) about the various compilation options, which you should review.

### Development Editor

The development editor should be the first thing you build. To do this, find your ThirdPersonMP.uproject file, right-click, and "Generate Visual Studio project files" (You may have already done this in the previous guide, in which case, you can skip this step. Once complete, you can open ThirdPersonMP.sln in Visual Studio.

Once Visual Studio loads, change your configuration to "Development Server". The Solution Configuration dropdown is sometimes hard to read, but can be resized in the toolbar customization options. Now, build. The first time you do this, it can take several hours, but you likely did this in the previous guide. Repeated builds should be fairly fast for a small project. Once finished, you can run/debug the project Solution/Games/ThirdPersonMP. This will start the Development Editor.

### Shipping Client

Run the Development Editor, and from the development editor window, pick these options in this order:

* File -> Package Project -> Build Target -> ThirdPersonMP (Or the name of your project)
* File -> Package Project -> Build Configuration -> Development
* File -> Package Project -> Windows (64-bit)
* (This opens a popup window asking you to pick a location - make sure you remember this location)
* {Wait a very long time}
	* The first shipping client build will take upto multiple hours, even if you've already built Development Editor
	* Repeat builds will be much faster for a simple project like ThirdPersonMP
* Open the client location you picked and find ThirdPersonMP.exe

### Development Server

* Close the Development Editor (Unreal is no longer running, only Visual Studio should be open now)
* Toolbars -> Configuration -> "Development Server"
* Build/Rebuild Solution
* {Wait a very long time}
	* The first development server build will take upto multiple hours, even if you've already built Development Editor
	* Repeat builds will be much faster for a simple project like ThirdPersonMP
* Use Explorer to find {projectLocation}\Binaries\Win64\ThirdPersonMPServer.exe

### Shipping Server

Run the Development Editor, and from the development editor window, pick these options in this order:

* File -> Package Project -> Build Target -> ThirdPersonMPServer (Or the name of your project)+Server
* File -> Package Project -> Build Configuration -> Development
* File -> Package Project -> Windows (64-bit)
* (This opens a popup window asking you to pick a location - make sure you remember this location)
* {Wait a very long time}
	* The first shipping server build will take upto multiple hours, even if you've already built Development Editor
	* Repeat builds will be much faster for a simple project like ThirdPersonMP
* Open the server location you picked and find ThirdPersonMPServer.exe

## Navigation

You are now ready to [test your game server](ThirdPersonMPLocalDeploy.md) on your local machine.

Alternately, you can return to the main [Unreal GSDK Plugin](README.md#deploy-to-playfab) guide.
