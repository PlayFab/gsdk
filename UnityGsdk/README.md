# GSDK for the Unity game engine

This folder contains the Game Server SDK for the Unity game engine. The GSDK files are in the `PlayFabSdk/MultiplayerAgent` folder. The entire GSDK is usable via class `PlayFabMultiplayerAgentAPI`. There is a sample scene in the `MultiplayerServerSample` folder that is meant to be used as a guidance when starting up a new project. For a more robust sample, check our `gsdksamples` repository [here](https://github.com/PlayFab/gsdkSamples/tree/master/UnityMirror).

### Can I use PlayFab SDK along the GSDK? 

Yup, PlayFab SDK and GSDK would reside in different folders in your Unity project, thus having no conflict.

### What do I need to do in order to use the Unity GSDK in a new project? 

You can drag the `PlayFabSDK` folder to your Unity project. After that, you need to enable the scripring directive `ENABLE_PLAYFABSERVER_API` on your Build settings, like in [this screenshot](https://user-images.githubusercontent.com/8256138/81462605-a6d7ac80-9168-11ea-9748-110ed01095c2.png)

## Release notes

- April 7th, 2020: This release of Unity GSDK removes the PlayFab SDK dependency. It keeps only the related SimpleJson serializer files. As we consider this release experimental, if you find any issues please [let us know by opening a new issue](https://github.com/PlayFab/gsdk/issues). Previous release can be found [here](https://github.com/PlayFab/gsdk/tree/2eb1178754950304385cc52ff4a3e32734807fb4).
