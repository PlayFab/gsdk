# Setting up a Linux Dedicated Server on PlayFab

The purpose of this guide is to describe all the steps needed to build and verify a Linux dedicated server. This was tested on both Unreal Engine 5 early access
and Unreal Engine 4.26 but the following steps will be specific to Unreal Engine 5, early access.

## Requirements
* Docker on Windows. If it is not installed, you can follow the instructions here: [Docker Desktop on Windows](https://docs.docker.com/desktop/windows/install/)

## Instructions

There is an [Unreal guide](https://docs.unrealengine.com/4.27/en-US/SharingAndReleasing/Linux/GettingStarted/) about the various cross - compilation options, which you should review. For Unreal Engine 5, early access, the -v17 Toolchain has been tested to work.

* Download the toolchain for your engine and run the executable. Please note that this will work only if your Unreal Engine build is from source.
* After the installation is complete, go to environment variables and make sure this exists on your system variables: **LINUX_MULTIARCH_ROOT**.
* In the directory of your source Unreal Engine build, run again Setup.bat and GenerateProjectFiles.bat.
* Now you can go back to your Unreal Engine project, build it in Development Editor mode and open the Unreal Editor.
* At this point, you should be able to package your project in Linux for both client and server in a directory of your choice. For Unreal Engine 5, in the Editor, click packaging on the top right of the screen and the Linux option should now be available to be selected.

## Create Docker Image of the Linux Server.

At this moment, in the directory chosen for packaging, you should be able to see a **Linux** directory if you packaged a Linux Client and a **LinuxServer** directory if you packaged a Linux Server, which is required for the following steps.
In a terminal of your choice, follow these steps:
* cd into the **LinuxServer** directory that was selected for packaging.
* Create a file named: dockerfile
* Edit the file with your editor of choice (vim, emacs etc.)
* Copy - paste the content presented below. Make sure to replace [YourGameNameServer].sh with the name of your linux server executable.

```bash
FROM ubuntu:18.04

# Unreal refuses to run as root user, so we must create a user to run as
# Docker uses root by default
RUN useradd --system ue
USER ue

EXPOSE 7777/udp

WORKDIR /server

COPY --chown=ue:ue . /server
CMD ./YourGameNameServer.sh
```


* Run: ```docker build -t "[imageName]:[tag]" . ```, where imageName and tag are options of your choice.
* Type **docker images** on the terminal to inspect the succesful creation of the docker image.

## Upload the Container Image on the Playfab Azure Registry
At this point, you are ready to push your docker image in the Playfab Azure Container Registry so that it can be used to launch your build on Playfab.

* Visit: [Playfab Multiplayer Servers](https://developer.playfab.com/en-US/r/t/59F84/multiplayer/server/builds) and go to your account.
* On the menu on the left side, click **Multiplayer** and on the new page, click **New Build** on the right of the screen.
* Under Server Details, set Server Type to Container and Operating System to Linux.
* Click the **Upload to container registry** button.
* On the window that pops up, click the **Copy Docker Login Command**.
* Paste the command on the terminal that was open for creating the docker image for the previous step. This will log you in the Azure Registry used by Playfab.
* On the terminal, type: ```docker tag [imageName]:[tag] customerzpqyvylj3p36i.azurecr.io/[imageName]:[tag]```, where imageName and tag where chosen in the previous step. If you are not sure, you can type **docker images** on the terminal and inspect them.
* Type: ```docker push customerzpqyvylj3p36i.azurecr.io/[imageName]:[tag]```
* Back on the Playfab website, under the Server Details on the New Build page, click Refresh next to Container image.
* Click the arrow button and you should be able to see and select the docker image you just uploaded with the respective tag.
* Fill in the rest of the required information on this page and you will be able to deploy your linux server build on PlayFab Multiplayer Servers.

