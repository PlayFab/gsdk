using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using PlayFab;
using System;
using PlayFab.ClientModels;
using Mirror;
using PlayFab.Helpers;

public class Startup : MonoBehaviour {
    PlayFabAuthService _authService;
    UnityNetworkingClient _unc;
    MessageWindow _messageWindow;

    // Use this for initialization
    void Start () {
        _authService = PlayFabAuthService.Instance;
        PlayFabAuthService.OnDisplayAuthentication += OnDisplayAuth;
        PlayFabAuthService.OnLoginSuccess += OnLoginSuccess;

        _unc = UnityNetworkingClient.Instance;
        _unc.OnDisconnected.AddListener(OnDisconnected);
        _unc.OnConnected.AddListener(OnConnected);
        NetworkClient.RegisterHandler(UnityNetworkingClient.CustomGameServerMessageTypes.ShutdownMessage, OnServerShutdown);
        NetworkClient.RegisterHandler(UnityNetworkingClient.CustomGameServerMessageTypes.MaintenanceMessage, OnMaintenanceMessage);

        _messageWindow = MessageWindow.Instance;
    }

    private void OnMaintenanceMessage(NetworkMessage netMsg)
    {
        var message = netMsg.ReadMessage<UnityNetworkingClient.MaintenanceMessage>();
        _messageWindow.Title.text = "Maintenance Shutown scheduled";
        _messageWindow.Message.text = string.Format("Maintenance is scheduled for: {0}", message.ScheduledMaintenanceUTC.ToString("MM-DD-YYYY hh:mm:ss"));
        _messageWindow.gameObject.SetActive(true);
    }

    private void OnServerShutdown(NetworkMessage netMsg)
    {
        _messageWindow.Title.text = "Shutdown In Progress";
        _messageWindow.Message.text = "Server has issued a shutdown.";
        _messageWindow.gameObject.SetActive(true);
        NetworkClient.Disconnect();
    }

    private void OnConnected()
    {
        _authService.Authenticate();
    }

    private void OnDisplayAuth()
    {
        _authService.Authenticate(Authtypes.Silent);
    }

    private void OnLoginSuccess(LoginResult success)
    {
        _messageWindow.Title.text = "Login Successful";
        _messageWindow.Message.text = string.Format("You logged in successfully. ID:{0}", success.PlayFabId);
        _messageWindow.gameObject.SetActive(true);

        NetworkClient.connection.Send(UnityNetworkingClient.CustomGameServerMessageTypes.ReceiveAuthenticate, new UnityNetworkingClient.ReceiveAuthenticateMessage()
        {
            PlayFabId = success.PlayFabId
        });
    }

    private void OnDisconnected(int? code)
    {
        _messageWindow.Title.text = "Disconnected!";
        _messageWindow.Message.text = "You were disconnected from the server";
        _messageWindow.gameObject.SetActive(true);
    }


}
