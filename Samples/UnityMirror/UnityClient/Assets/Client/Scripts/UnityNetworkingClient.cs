using PlayFab;
using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Mirror;
using UnityEngine.Events;

public class UnityNetworkingClient : MonoBehaviour {
    public ConnectedEvent OnConnected = new ConnectedEvent();
    public DisconnectedEvent OnDisconnected = new DisconnectedEvent();

    public NetworkClient Client;
    private NetworkManager _netManager;

    public class ConnectedEvent : UnityEvent { }
    public class DisconnectedEvent : UnityEvent<int?> { }

    public static UnityNetworkingClient Instance { get; set; }

    private void Awake()
    {
        Instance = this;

        _netManager = GetComponent<NetworkManager>();
        _netManager.StartClient();
        Client = _netManager.client;
    }

    private void Start()
    {
        NetworkClient.RegisterHandler(MsgType.Connect, OnNetworkConnected);
        NetworkClient.RegisterHandler(MsgType.Disconnect, OnNetworkDisconnected);
    }

    private void OnNetworkConnected(NetworkMessage netMsg)
    {
        Debug.Log("Connected To Network Server");
        OnConnected.Invoke();
    }

    private void OnNetworkDisconnected(NetworkMessage netMsg)
    {
        ErrorMessage message = new ErrorMessage();
        try
        {
            message = netMsg.ReadMessage<ErrorMessage>();
        }
        catch (Exception e) { }

        if(message.value != 0){
            OnDisconnected.Invoke(message.value);
        }
        else
        {
            OnDisconnected.Invoke(null);
        }
    }

    public class CustomGameServerMessageTypes
    {
        public const short ReceiveAuthenticate = 900;
        public const short ShutdownMessage = 901;
        public const short MaintenanceMessage = 902;
    }

    public class ReceiveAuthenticateMessage : MessageBase
    {
        public string PlayFabId;
    }

    public class ShutdownMessage : MessageBase { }

    [Serializable]
    public class MaintenanceMessage : MessageBase
    {
        public DateTime ScheduledMaintenanceUTC;

        public override void Deserialize(NetworkReader reader)
        {
            var json = PlayFab.PluginManager.GetPlugin<ISerializerPlugin>(PluginContract.PlayFab_Serializer);
            ScheduledMaintenanceUTC = json.DeserializeObject<DateTime>(reader.ReadString());
        }

        public override void Serialize(NetworkWriter writer)
        {
            var json = PlayFab.PluginManager.GetPlugin<ISerializerPlugin>(PluginContract.PlayFab_Serializer);
            var str = json.SerializeObject(ScheduledMaintenanceUTC);
            writer.Write(str);
        }
    }

}
