# PlayFab Multiplayer Unity Custom Game Server Sample

## Overview

This repository is the home of a custom game server example, made in unity3d, that shows how to create a game server that can talk to PlayFab's new multiplayer platform.   There are a couple key components such as the MultiPlayer Agent and a sample game client in this repository.  The purpose of this project is to provide you with an out-of-box example that you, as a game developer, can use as a starting point for your own game server.  In addition, we will identify in this document key parts of this project so that you can integrate what is needed into an existing project.

## Prerequisites

- Unity Engine (tested with 2019.2.18f1)
- Understanding of C# language
- Basic or Intermediate knowledge of Unity
- Recent version of the [PlayFab Game Server SDK](https://github.com/PlayFab/gsdk/tree/master/UnityGsdk)
- When running locally you must have a copy of [Local Multiplayer Agent](https://github.com/PlayFab/LocalMultiplayerAgent)

## gsdk - Game Server SDK (aka Multiplayer Agent API)

Your game server uses the GSDK to talk to the Agent process that is running inside the Virtual Machine. The Agent's purpose is to communicate the game server status to the Multiplayer Servers service. Agent can also send notifications to game server to notify of shutdowns and maintenance schedules.

## Included Projects

There are two projects included in this repository. 
- UnityServer -  This is an example game server project that can talk to the Multiplayer Agent.  
- UnityClient - This is an example client that can talk to the UnityServer 

## Networking API

This sample uses [Mirror](https://github.com/vis2k/Mirror), the community replacement for Unity's UNET Networking System. You can see Mirror's documentation [here](https://mirror-networking.com/docs/General/index.html).

## Getting Started for Windows game servers using Windows Containers

1. Make sure you have Docker for Windows installed, check [here](https://docs.docker.com/docker-for-windows/install/) for installation instructions. You should have configured it to use Windows Containers. 
1. Download the repository
1. Open the two projects. 
1. Open the folder where you have downloaded the *LocalMultiplayerAgent* and modify the *MultiplayerSettings.json* file. Set the following values: 
    - RunContainers = true
    - OutputFolder = A local path with enough space on your machine. Useful data for your game will be written there.
    - LocalFilePath = Path to a zipped folder of the build output for the server project. Something like: F:\\MPSUnitySample\\Output\\game.zip
    - StartGameCommand = C:\\Assets\\UnityServer.exe -batchmode -logFile C:\\GameLogs\\UnityEditor.log
    - Within GamePort, set Number to 7777 and Protocol to TCP. Also make a note of the external port number, called NodePort
1. Build the server (UnityServer) project. Compress the entire output and save it to a file (zip format), which path corresponds to the *LocalFilePath* that you edited above. When compressing bear in mind that you should not compress the folder containing the Unity output, just the files themselves.
1. In PowerShell
    - Run the LocalMultiplayerAgentSetup file in *agentfolder/setup.ps1* (you may need to open Powershell with admin permissions for this purpose)
    - Run the Local Multiplayer Agent located in *agentfolder/MockVmAgent.exe*. Observe the output messages from the Agent and that the game is in *StandingBy* state.
1. As soon as your Agent is receiving heartbeats, you can Build/Run or Run inside Editor the client (UnityClient) project. Observe that the game is in *Active* state. When the server finishes execution, your client will disconnect as well.

## PlayFabMulitiplayerAgent API reference

**Start()** -  This initializes the GameObject into the scene that will talk to the Agent.  It also uses environment variables to know how to talk to the local agent

**ReadyForPlayers()** -- This tells the agent that your server is ready for Players.  Under the hood it is setting the agent current status to  StandingBy.  You typically want to call this once all your own initialization code has been started.  The server will stay in this state until the Multiplayer agent Service moves to Active.

**SetState(SessionHostStatus status)** -  This api tells the mutliplayer agent service that you are changing the state on the next heartbeat.  For example, if you are shutting down the game server, you can tell the agent and it will move to terminating status.

**AddPlayer(string playerId)** - Informs the multiplayer agent that a player has been added to the server.  PlayerId is arbitrary, but most common is to use the PlayFabId or EntityId of the player added.

**RemovePlayer(string playerId)** - Informs the multiplayer agent that a player has been removed or left the server.  PlayerId is arbitrary, but most common is to use the PlayFabId or EntityId of the player removed.

**GetConfigSettings()** -- Returns an object with a series of configuration values available to your server. Properties include the PlayFab TitleId, the ServerId, the Region for this server, etc.

<hr/>

## GSDK (MultiplayerAgentAPI) - Events

**OnMaintenanceCallback** - Listen to this event to get updates about when the server has scheduled maintenance.

**OnShutDownCallback** - Listen to this event to know when the server is shutting down

**OnServerActiveCallback** - Listen to this event to know when the state of the server has been moved to Active.  In general, you will want to startup your networking service layer when this event has been fired.   When active, that means you are truely ready to receive player connections.

**OnAgentErrorCallback** - If any error happens while trying to speak to the local agent

<hr/>

## Components

### AgentListener

In the code below, you'll notice that we are subscribing to events from the GSDK. 

We are doing the following tasks with this componenet

1. Start (init) the Agent API -  this means we are now talking to the agent every N seconds.
2. Listen for events
3. Notify the server that we are ready for players ( in standby mode )
4. When we receive a status of Active, we are starting the unity networking server.
5. Messages received are sent to connected clients


 ``` c#
using System.Collections;
using UnityEngine;
using PlayFab;
using System;
using PlayFab.Networking;

public class AgentListener : MonoBehaviour {
    public UnityNetworkServer UNetServer;
    public bool Debugging = false;
    // Use this for initialization
    void Start () {
        _connectedPlayers = new List<ConnectedPlayer>();
        PlayFabMultiplayerAgentAPI.Start();
        PlayFabMultiplayerAgentAPI.IsDebugging = Debugging;
        PlayFabMultiplayerAgentAPI.OnMaintenanceCallback += OnMaintenance;
        PlayFabMultiplayerAgentAPI.OnShutDownCallback += OnShutdown;
        PlayFabMultiplayerAgentAPI.OnServerActiveCallback += OnServerActive;
        PlayFabMultiplayerAgentAPI.OnAgentErrorCallback += OnAgentError;

        UNetServer.OnPlayerAdded.AddListener(OnPlayerAdded);
        UNetServer.OnPlayerRemoved.AddListener(OnPlayerRemoved);

        StartCoroutine(ReadyForPlayers());

    }

    IEnumerator ReadyForPlayers()
    {
        yield return new WaitForSeconds(.5f);
        PlayFabMultiplayerAgentAPI.ReadyForPlayers();
    }
    
    private void OnServerActive()
    {
        UNetServer.StartServer();
        Debug.Log("Server Started From Agent Activation");
    }

    private void OnPlayerRemoved(string playfabId)
    {
        ConnectedPlayer player = _connectedPlayers.Find(x => x.PlayerId.Equals(playfabId, StringComparison.OrdinalIgnoreCase));
        _connectedPlayers.Remove(player);
        PlayFabMultiplayerAgentAPI.UpdateConnectedPlayers(_connectedPlayers);
    }

    private void OnPlayerAdded(string playfabId)
    {
        _connectedPlayers.Add(new ConnectedPlayer(playfabId));
        PlayFabMultiplayerAgentAPI.UpdateConnectedPlayers(_connectedPlayers);
    }

    private void OnAgentError(string error)
    {
        Debug.Log(error);
    }

    private void OnShutdown()
    {
        Debug.Log("Server is shutting down");
        foreach(var conn in UNetServer.Connections)
        {
            conn.Connection.Send(CustomGameServerMessageTypes.ShutdownMessage, new ShutdownMessage());
        }
        StartCoroutine(Shutdown());
    }

    IEnumerator Shutdown()
    {
        yield return new WaitForSeconds(5f);
        Application.Quit();
    }

    private void OnMaintenance(DateTime? NextScheduledMaintenanceUtc)
    {
        Debug.LogFormat("Maintenance scheduled for: {0}", NextScheduledMaintenanceUtc.Value.ToLongDateString());
        foreach (var conn in UNetServer.Connections)
        {
            conn.Connection.Send(CustomGameServerMessageTypes.ShutdownMessage, new MaintenanceMessage() {
                ScheduledMaintenanceUTC = (DateTime)NextScheduledMaintenanceUtc
            });
        }
    }
}
 ```
