#if ENABLE_PLAYFABSERVER_API && ENABLE_PLAYFABAGENT_API
using PlayFab.Internal;
using System;
using System.Collections.Generic;
using System.IO;
using UnityEngine;

namespace PlayFab
{
    using AgentModels;
    using System.Text;
    
    #pragma warning disable 414
    public class PlayFabAgentAPI
    {

        private static readonly string HEARTBEAT_ENDPOINT_VARIABLE_NAME = "HEARTBEAT_ENDPOINT";
        private static readonly string SERVER_ID_VARIABLE_NAME = "SESSION_HOST_ID";
        private static readonly string LOG_FOLDER_VARIABLE_NAME = "GSDK_LOG_FOLDER";

        private static string _endpoint = "localhost:56001";
        private static string baseURL = string.Empty;
        private static string logFolder = string.Empty;
        private static string sessionId = string.Empty;
        private static SessionCookie cookie = new SessionCookie();
        private static ISerializerPlugin _jsonWrapper;
        

        public delegate void OnShutdownEvent();
        public static event OnShutdownEvent OnShutDown;

        public delegate void OnMaintenanceEvent(DateTime? NextScheduledMaintenanceUtc);
        public static event OnMaintenanceEvent OnMaintenance;

        public delegate void OnAgentCommunicationErrorEvent(string error);
        public static event OnAgentCommunicationErrorEvent OnAgentError;

        public static SessionHostHeartbeatInfo CurrentState = new SessionHostHeartbeatInfo();
        public static ErrorStates CurrentErrorState = ErrorStates.Ok;
        public static bool IsProcessing = false;
        public static bool IsDebugging = false;

        public static void Init()
        {
            _jsonWrapper = PlayFab.PluginManager.GetPlugin<ISerializerPlugin>(PluginContract.PlayFab_Serializer);
            _endpoint = Environment.GetEnvironmentVariable(HEARTBEAT_ENDPOINT_VARIABLE_NAME);
            sessionId = Environment.GetEnvironmentVariable(SERVER_ID_VARIABLE_NAME);
            logFolder = Environment.GetEnvironmentVariable(LOG_FOLDER_VARIABLE_NAME);
            baseURL = string.Format("http://{0}/v1/sessionHosts/{1}/heartbeats", _endpoint, sessionId);
            CurrentState.currentGameState = SessionHostStatus.Initializing;
            CurrentErrorState = ErrorStates.Ok;

            if (IsDebugging)
            {
                Debug.Log(baseURL);
                Debug.Log(sessionId);
                Debug.Log(logFolder);
            }

            //Create an agent that can talk on the main-tread and pull on an interval.
            //This is a unity thing, need an object in the scene.
            GameObject agentView = new GameObject("PlayFabAgentView");
            agentView.AddComponent<PlayFabAgentView>();
        }

        public static void SetState(SessionHostStatus status)
        {
            CurrentState.currentGameState = status;
        }

        public static void AddPlayer(string playerId)
        {
            if (CurrentState.currentPlayers.Find(p => p.PlayerId == playerId) != null)
            {
                CurrentState.currentPlayers.Add(new ConnectedPlayer() { PlayerId = playerId });
            }
        }

        public static void RemovePlayer(string playerId)
        {
            var player = CurrentState.currentPlayers.Find(p => p.PlayerId == playerId);
            if (player != null)
            {
                CurrentState.currentPlayers.Remove(player);
            }
        }

        internal static void SendHeartBeatRequest()
        {
            
            var payload = _jsonWrapper.SerializeObject(CurrentState);
            if (string.IsNullOrEmpty(payload)) { return; }
            var payloadBytes = Encoding.ASCII.GetBytes(payload);
    
            PlayFabHttp.SimplePostCall(baseURL, payloadBytes, (success) => {
                var json = System.Text.Encoding.UTF8.GetString(success);
                Debug.Log(json);
                if (string.IsNullOrEmpty(json)) { return;  }
                var hb = _jsonWrapper.DeserializeObject<SessionHostHeartbeatInfo>(json);
                if(hb != null)
                {
                    ProcessAgentResponse(hb);
                }
                CurrentErrorState = ErrorStates.Ok;
                IsProcessing = false;
            }, (error) => {

                var guid = Guid.NewGuid();
                Debug.LogFormat("CurrentError: {0} - {1}", error, guid.ToString());
                //Exponential backoff for 30 minutes for retries.
                switch (CurrentErrorState)
                {
                    case ErrorStates.Ok:
                        CurrentErrorState = ErrorStates.Retry30s;
                        if (IsDebugging)
                            Debug.Log("Retrying heartbeat in 30s");
                        break;
                    case ErrorStates.Retry30s:
                        CurrentErrorState = ErrorStates.Retry5m;
                        if (IsDebugging)
                            Debug.Log("Retrying heartbeat in 5m");
                        break;
                    case ErrorStates.Retry5m:
                        CurrentErrorState = ErrorStates.Retry10m;
                        if (IsDebugging)
                            Debug.Log("Retrying heartbeat in 10m");
                        break;
                    case ErrorStates.Retry10m:
                        CurrentErrorState = ErrorStates.Retry15m;
                        if (IsDebugging)
                            Debug.Log("Retrying heartbeat in 15m");
                        break;
                    case ErrorStates.Retry15m:
                        CurrentErrorState = ErrorStates.Cancelled;
                        if (IsDebugging)
                            Debug.Log("Agent reconnection cannot be established - cancelling");
                        break;
                }

                if(OnAgentError != null)
                {
                    OnAgentError.Invoke(error);
                }
                IsProcessing = false;
            });
        }

        public static void ProcessAgentResponse(SessionHostHeartbeatInfo heartBeat)
        {

            if (heartBeat.sessionConfig != null && !string.IsNullOrEmpty(heartBeat.sessionConfig.SessionCookie))
            {
                cookie = _jsonWrapper.DeserializeObject<SessionCookie>(heartBeat.sessionConfig.SessionCookie);
            }

            if(heartBeat.nextScheduledMaintenanceUtc != null)
            {
                if(OnMaintenance != null)
                {
                    OnMaintenance.Invoke(heartBeat.nextScheduledMaintenanceUtc);
                }
            }

            switch (heartBeat.operation)
            {
                case Operation.Continue:
                    //No Action Required.
                    break;
                case Operation.Active:
                    //Transition Server State to Active.
                    CurrentState.currentGameState = SessionHostStatus.Active;
                    break;
                case Operation.Terminate:
                    //Transition Server to a Termination state.
                    CurrentState.currentGameState = SessionHostStatus.Terminating;
                    if(OnShutDown != null)
                    {
                        OnShutDown.Invoke();
                    }
                    break;
                default:
                    Debug.LogWarning("Unknown operation received" + heartBeat.operation);
                    break;
            }

            if (IsDebugging)
                Debug.LogFormat("Operation: {0}, Maintenance:{1}, State: {2}", heartBeat.operation, heartBeat.nextScheduledMaintenanceUtc.ToString(), heartBeat.currentGameState);
        }

    }

    public class PlayFabAgentView : MonoBehaviour
    {
        private float _timer = 0f;
        private void LateUpdate()
        {
            var max = PlayFabAgentAPI.CurrentState != null && PlayFabAgentAPI.CurrentState.nextHeartbeatIntervalMs != null ? (float)(PlayFabAgentAPI.CurrentState.nextHeartbeatIntervalMs / 1000) : 1f;
            _timer += Time.deltaTime;
            if (PlayFabAgentAPI.CurrentErrorState != ErrorStates.Ok)
            {
                switch (PlayFabAgentAPI.CurrentErrorState)
                {
                    case ErrorStates.Retry30s:
                    case ErrorStates.Retry5m:
                    case ErrorStates.Retry10m:
                    case ErrorStates.Retry15m:
                        max = (float)PlayFabAgentAPI.CurrentErrorState;
                        break;
                    case ErrorStates.Cancelled:
                        max = 1f;
                        break;
                }
            }

            if (PlayFabAgentAPI.CurrentErrorState != ErrorStates.Cancelled && !PlayFabAgentAPI.IsProcessing && _timer >= max)
            {
                if (PlayFabAgentAPI.IsDebugging)
                {
                    Debug.LogFormat("Timer:{0} - Max:{1}", _timer, max);
                }
                PlayFabAgentAPI.IsProcessing = true;
                _timer = 0f;
                PlayFabAgentAPI.SendHeartBeatRequest();
                
            }
        }
    }
}

namespace PlayFab.AgentModels
{
    /*
    [Serializable]
    public class SessionHostHeartbeatInfo
    {
        public SessionHostStatus currentGameState; // Request 
        public bool isGameHealthy; // Request (TODO - HARO UPDATE THIS IN CONTROL PLANE) 
        public List<ConnectedPlayer> currentPlayers = new List<ConnectedPlayer>(); // Request 

        public int? nextHeartbeatIntervalMs; // Response 
        public Operation? operation; // Response 
        public SessionConfig sessionConfig = new SessionConfig(); // Response (only set on Allocation, when switching to Active) 
        public List<PortMapping> portMappings = new List<PortMapping>(); // Response 
        public DateTime? nextScheduledMaintenanceUtc; // Response 
        public string assignmentId; // Response 

    }
    */

    [Serializable]
    public class SessionHostHeartbeatInfo
    {
        /// <summary>
        /// The current game state. For example - StandingBy, Active etc.
        /// </summary>
        public SessionHostStatus currentGameState { get; set; }

        /// <summary>
        /// The number of milliseconds to wait before sending the next heartbeat.
        /// </summary>
        public int? nextHeartbeatIntervalMs { get; set; }

        public Operation? operation { get; set; }

        /// <summary>
        /// The game host's current health.
        /// </summary>
        public SessionHostHealth currentGameHealth { get; set; }

        /// <summary>
        /// List of players connected to the game host.
        /// </summary>
        public List<ConnectedPlayer> currentPlayers { get; set; }

        /// <summary>
        /// The time at which the <see cref="CurrentGameState"/> had last changed.
        /// </summary>
        public DateTime? lastStateTransitionTimeUtc { get; set; }

        /// <summary>
        /// The configuration sent down to the game host from Control Plane.
        /// </summary>
        public SessionConfig sessionConfig { get; set; }

        /// <summary>
        /// The port mappings used by thsi session host.
        /// </summary>
        public List<PortMapping> portMappings { get; set; }

        /// <summary>
        /// The next scheduled maintenance time from Azure, in UTC.
        /// </summary>
        public DateTime? nextScheduledMaintenanceUtc { get; set; }

        /// <summary>
        /// Identifies the title, deployment and region for this particular session host.
        /// This could differ among the session hosts running on a VM that is running multiple deployments.
        /// </summary>
        public string assignmentId { get; set; }
    }

    [Serializable]
    public enum SessionHostStatus

    {
        Invalid, //TODO: Haro find out if we use this
        Initializing,
        StandingBy,
        Active,
        Terminating,
        Terminated,
        Quarantined,
        PendingAllocation, //TODO: Haro find out if we use this
        AllocationTimeout //TODO: Haro find out if we use this
    }

    [Serializable]
    public enum Operation
    {
        Invalid,
        Continue,
        GetManifest, 
        Quarantine,
        Active,
        Terminate
    }

    [Serializable]
    public enum SessionHostHealth
    {
        Unhealthy,
        Healthy,
    }

    [Serializable]
    public class ConnectedPlayer
    {
        public string PlayerId { get; set; }
    }

    [Serializable]
    public class SessionConfig
    {
        public string SessionId { get; set; }
        public string SessionCookie { get; set; }
    }

    [Serializable]
    public class SessionCookie
    {
        public string LobbyId;
        public string TitleId;
        public string EnvTag;
        public string EnvGroup;
        public string LobbyServiceTitleName;
        public string CmsPath;
        public string CmsTitleName;
    }

    [Serializable]
    public class PortMapping
    {
        public int PublicPort { get; set; }
        public int NodePort { get; set; }
        public Port GamePort { get; set; }
    }

    [Serializable]
    public class Port
    {
        public string Name { get; set; }
        public int Number { get; set; }
        public string Protocol { get; set; }
    }
    
    [Serializable]
    public enum ErrorStates
    {
        Ok = 0,
        Pending = 1,
        Retry30s = 30,
        Retry5m = 300,
        Retry10m = 600,
        Retry15m = 900,
        Cancelled = -1
    }

}

#endif