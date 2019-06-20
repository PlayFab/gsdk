using System;
using System.Collections;
using System.Collections.Generic;
using PlayFab;
using PlayFab.MultiplayerAgent.Model;
using PlayFab.Networking;
using UnityEngine;

public class MultiplayerGameServer : MonoBehaviour
{
    private List<ConnectedPlayer> _connectedPlayers;
    public bool Debugging = false;

    public UnityNetworkServer UNetServer;

    // Use this for initialization
    private void Start()
    {
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

    private IEnumerator ReadyForPlayers()
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
        Debug.Log("Server is Shutting down");
        foreach (UnityNetworkConnection conn in UNetServer.Connections)
        {
            conn.Connection.Send(CustomGameServerMessageTypes.ShutdownMessage, new ShutdownMessage());
        }

        StartCoroutine(Shutdown());
    }

    private IEnumerator Shutdown()
    {
        yield return new WaitForSeconds(5f);
        Application.Quit();
    }

    private void OnMaintenance(DateTime? NextScheduledMaintenanceUtc)
    {
        Debug.LogFormat("Maintenance Scheduled for: {0}", NextScheduledMaintenanceUtc.Value.ToLongDateString());
        foreach (UnityNetworkConnection conn in UNetServer.Connections)
        {
            conn.Connection.Send(CustomGameServerMessageTypes.ShutdownMessage, new MaintenanceMessage
            {
                ScheduledMaintenanceUTC = (DateTime) NextScheduledMaintenanceUtc
            });
        }
    }
}