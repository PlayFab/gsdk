namespace PlayFab.Networking
{
    using System;
    using System.Collections.Generic;
    using UnityEngine;
    using Mirror;
    using UnityEngine.Events;

    public class UnityNetworkServer : MonoBehaviour
    {
        public PlayerEvent OnPlayerAdded = new PlayerEvent();
        public PlayerEvent OnPlayerRemoved = new PlayerEvent();

        public int MaxConnections = 100;
        public int Port = 7777;

        private NetworkManager _netManager;

        public List<UnityNetworkConnection> Connections {
            get { return _connections; }
            private set { _connections = value; }
        }
        private List<UnityNetworkConnection> _connections = new List<UnityNetworkConnection>();

        public class PlayerEvent : UnityEvent<string> { }

        // Use this for initialization
        void Awake()
        {
            _netManager = FindObjectOfType<NetworkManager>();
            NetworkServer.RegisterHandler(MsgType.Connect, OnServerConnect);
            NetworkServer.RegisterHandler(MsgType.Disconnect, OnServerDisconnect);
            NetworkServer.RegisterHandler(MsgType.Error, OnServerError);
            NetworkServer.RegisterHandler(CustomGameServerMessageTypes.ReceiveAuthenticate, OnReceiveAuthenticate);
            //_netManager.transport.port = Port;
        }

        public void StartServer()
        {
            NetworkServer.Listen(Port);
        }
        
        private void OnApplicationQuit()
        {
            NetworkServer.Shutdown();
        }

        private void OnReceiveAuthenticate(NetworkMessage netMsg)
        {
            var conn = _connections.Find(c => c.ConnectionId == netMsg.conn.connectionId);
            if(conn != null)
            {
                var message = netMsg.ReadMessage<ReceiveAuthenticateMessage>();
                conn.PlayFabId = message.PlayFabId;
                conn.IsAuthenticated = true;
                OnPlayerAdded.Invoke(message.PlayFabId);
            }
        }

        private void OnServerConnect(NetworkMessage netMsg)
        {
            Debug.LogWarning("Client Connected");
            var conn = _connections.Find(c => c.ConnectionId == netMsg.conn.connectionId); 
            if(conn == null)
            {
                _connections.Add(new UnityNetworkConnection()
                {
                    Connection = netMsg.conn,
                    ConnectionId = netMsg.conn.connectionId,
                    LobbyId = PlayFabMultiplayerAgentAPI.SessionConfig.SessionId
                });
            }
        }

        private void OnServerError(NetworkMessage netMsg)
        {
            try
            {
                var error = netMsg.ReadMessage<ErrorMessage>();
                if (error.value != 0)
                {
                    Debug.Log(string.Format("Unity Network Connection Status: code - {0}", error.value));
                }
            }
            catch (Exception)
            {
                Debug.Log("Unity Network Connection Status, but we could not get the reason, check the Unity Logs for more info.");
            }
        }

        private void OnServerDisconnect(NetworkMessage netMsg)
        {
            var conn = _connections.Find(c => c.ConnectionId == netMsg.conn.connectionId);
            if(conn != null)
            {
                if (!string.IsNullOrEmpty(conn.PlayFabId))
                {
                    OnPlayerRemoved.Invoke(conn.PlayFabId);
                }
                _connections.Remove(conn);
            }
        }

    }

    [Serializable]
    public class UnityNetworkConnection
    {
        public bool IsAuthenticated;
        public string PlayFabId;
        public string LobbyId;
        public int ConnectionId;
        public NetworkConnection Connection;
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

    public class ShutdownMessage : MessageBase {}

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