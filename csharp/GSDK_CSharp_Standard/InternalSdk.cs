using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;

namespace Microsoft.Playfab.Gaming.GSDK.CSharp
{
    class InternalSdk : IDisposable
    {
        private string _overrideConfigFileName;
        private GameState _state;

        private Task _heartbeatTask;
        private Configuration _configuration;
        private IHttpClient _webClient;
        private bool _heartbeatRunning;
        private DateTime _cachedScheduleMaintDate;
        private ManualResetEvent _heartbeatDoneEvent = new ManualResetEvent(false);
        private ManualResetEvent _signalHeartbeatEvent = new ManualResetEvent(false);
        private bool _debug;

        public ManualResetEvent TransitionToActiveEvent { get; set; }
        public IDictionary<string, string> ConfigMap { get; private set; }
        public ILogger Logger { get; private set; }
        public GameState State
        {
            get { return _state; }
            set
            {
                if (_state != value)
                {
                    _state = value;
                    _signalHeartbeatEvent.Set();
                }
            }
        }

        public IList<ConnectedPlayer> ConnectedPlayers { get; set; }

        public IList<string> InitialPlayers { get; set; }

        // Keep an instance of the callbacks around so they don't get garbage collected
        public Action ShutdownCallback { get; set; }
        public Func<bool> HealthCallback { get; set; }
        public Action<DateTimeOffset> MaintenanceCallback { get; set; }


        public InternalSdk(string configFileName = null)
        {
            _overrideConfigFileName = configFileName;
            ConnectedPlayers = new List<ConnectedPlayer>();
            InitialPlayers = new List<string>();
            TransitionToActiveEvent = new ManualResetEvent(false);
        }

        public Task StartAsync(bool debugLogs = false)
        {
            // If we already initialized everything, no need to do it again
            if (_heartbeatTask != null)
            {
                return Task.CompletedTask;
            }

            _debug = debugLogs;


            if (_configuration == null)
            {
                _configuration = GetConfiguration();
            }

            if (ConfigMap == null)
            {
                ConfigMap = CreateConfigMap(_configuration);
            }

            Logger = LoggerFactory.CreateInstance(ConfigMap[GameserverSDK.LogFolderKey]);
            Logger.Start();
            

            string gsmsBaseUrl = ConfigMap[GameserverSDK.HeartbeatEndpointKey];
            string instanceId = ConfigMap[GameserverSDK.ServerIdKey];

            Logger.Log($"VM Agent Endpoint: {gsmsBaseUrl}");
            Logger.Log($"Instance Id: {instanceId}");

            _webClient = HttpClientFactory.CreateInstance($"http://{gsmsBaseUrl}/v1/sessionHosts/{instanceId}");

            _heartbeatRunning = true;
            _signalHeartbeatEvent.Reset();
            TransitionToActiveEvent.Reset();

            _heartbeatTask = Task.Run(HeartbeatAsync);
            return Task.CompletedTask;
        }

        private Configuration GetConfiguration()
        {
            string fileName;

            if (!string.IsNullOrWhiteSpace(_overrideConfigFileName))
            {
                fileName = _overrideConfigFileName;
            }
            else
            {
                fileName = Environment.GetEnvironmentVariable(GameserverSDK.GsdkConfigFileEnvVarKey);
            }

            Configuration localConfig;

            if (!string.IsNullOrWhiteSpace(fileName) && File.Exists(fileName))
            {
                localConfig = new JsonFileConfiguration(fileName);
            }
            else
            {
                localConfig = new EnvironmentVariableConfiguration();
            }

            return localConfig;
        }

        private IDictionary<string, string> CreateConfigMap(Configuration localConfig)
        {
            var finalConfig = new Dictionary<string, string>();

            foreach (KeyValuePair<string, string> certEntry in localConfig.GameCertificates)
            {
                finalConfig[certEntry.Key] = certEntry.Value;
            }

            foreach (KeyValuePair<string, string> metadata in localConfig.BuildMetadata)
            {
                finalConfig[metadata.Key] = metadata.Value;
            }

            foreach (KeyValuePair<string, string> port in localConfig.GamePorts)
            {
                finalConfig[port.Key] = port.Value;
            }

            finalConfig[GameserverSDK.HeartbeatEndpointKey] = localConfig.HeartbeatEndpoint;
            finalConfig[GameserverSDK.ServerIdKey] = localConfig.ServerId;
            finalConfig[GameserverSDK.VmIdKey] = localConfig.VmId;
            finalConfig[GameserverSDK.LogFolderKey] = localConfig.LogFolder;
            finalConfig[GameserverSDK.SharedContentFolderKey] = localConfig.SharedContentFolder;
            finalConfig[GameserverSDK.CertificateFolderKey] = localConfig.CertificateFolder;
            finalConfig[GameserverSDK.TitleIdKey] = localConfig.TitleId;
            finalConfig[GameserverSDK.BuildIdKey] = localConfig.BuildId;
            finalConfig[GameserverSDK.RegionKey] = localConfig.Region;
            finalConfig[GameserverSDK.PublicIpV4AddressKey] = localConfig.PublicIpV4Address;
            finalConfig[GameserverSDK.FullyQualifiedDomainNameKey] = localConfig.FullyQualifiedDomainName;

            return finalConfig;
        }

        private async Task HeartbeatAsync()
        {
            while (_heartbeatRunning)
            {
                if (_signalHeartbeatEvent.WaitOne(1000))
                {
                    if (_debug)
                    {
                        Logger.Log("State transition signaled an early heartbeat.");
                    }

                    _signalHeartbeatEvent.Reset();
                }

                await SendHeartbeatAsync();
            }

            Logger.Log("Shutting down heartbeat thread");
            _heartbeatDoneEvent.Set();
        }

        internal async virtual Task SendHeartbeatAsync()
        {
            bool gameHealth = false;
            if (HealthCallback != null)
            {
                gameHealth = HealthCallback();
            }

            var payload = new HeartbeatRequest
            {
                CurrentGameState = this.State,
                CurrentGameHealth = gameHealth ? "Healthy" : "Unhealthy",
                CurrentPlayers = ConnectedPlayers.ToArray(),
            };

            try
            {
                HeartbeatResponse response = await _webClient.SendHeartbeatAsync(payload);
                await UpdateStateFromHeartbeatAsync(response);

                if (_debug)
                {
                    Logger.Log($"Heartbeat request: {{ state = {payload.CurrentGameState} }} response: {{ operation = {response.Operation} }}");
                }
            }
            catch (Exception ex)
            {
                Logger.Log($"Cannot send heartbeat: {ex.Message}\r\n\r\n{ex}");
                return;
            }
        }

        private Task UpdateStateFromHeartbeatAsync(HeartbeatResponse response)
        {
            if (response.SessionConfig != null)
            {
                ConfigMap.AddIfNotNullOrEmpty(GameserverSDK.SessionCookieKey, response.SessionConfig?.SessionCookie);
                ConfigMap.AddIfNotNullOrEmpty(GameserverSDK.SessionIdKey, response.SessionConfig?.SessionId.ToString());

                if (response.SessionConfig?.InitialPlayers != null && response.SessionConfig.InitialPlayers.Any())
                {
                    InitialPlayers = response.SessionConfig.InitialPlayers;
                }
            }

            if (!string.IsNullOrWhiteSpace(response.NextScheduledMaintenanceUtc))
            {
                if (DateTime.TryParse(
                    response.NextScheduledMaintenanceUtc,
                    null,
                    DateTimeStyles.RoundtripKind,
                    out DateTime scheduledMaintDate))
                {
                    if (_cachedScheduleMaintDate == DateTime.MinValue || (scheduledMaintDate != _cachedScheduleMaintDate))
                    {
                        MaintenanceCallback?.Invoke(scheduledMaintDate);
                        _cachedScheduleMaintDate = scheduledMaintDate;
                    }
                }
            }

            switch (response.Operation)
            {
                case GameOperation.Continue:
                    {
                        // No action required
                        break;
                    }
                case GameOperation.Active:
                    {
                        if (State != GameState.Active)
                        {
                            State = GameState.Active;
                        }

                        TransitionToActiveEvent.Set();

                        break;
                    }
                case GameOperation.Terminate:
                    {
                        if (State != GameState.Terminating)
                        {
                            State = GameState.Terminating;
                            ShutdownCallback?.Invoke();
                        }

                        TransitionToActiveEvent.Set();

                        break;
                    }
                default:
                    {
                        Logger.Log($"Unknown operation received: {Enum.GetName(typeof(GameOperation), response.Operation)}");
                        break;
                    }

            }

            return Task.CompletedTask;
        }

        #region IDisposable Support
        private bool disposedValue = false; // To detect redundant calls

        private void DisposeManagedResources(bool disposing)
        {
            if (!disposedValue)
            {
                if (disposing)
                {
                    _heartbeatRunning = false;
                    _heartbeatDoneEvent.WaitOne();
                }

                // TODO: free unmanaged resources (unmanaged objects) and override a finalizer below.
                // TODO: set large fields to null.

                disposedValue = true;
            }
        }

        // TODO: override a finalizer only if Dispose(bool disposing) above has code to free unmanaged resources.
        // ~InternalSdk() {
        //   // Do not change this code. Put cleanup code in Dispose(bool disposing) above.
        //   Dispose(false);
        // }

        // This code added to correctly implement the disposable pattern.
        public void Dispose()
        {
            // Do not change this code. Put cleanup code in Dispose(bool disposing) above.
            DisposeManagedResources(true);
            // TODO: uncomment the following line if the finalizer is overridden above.
            // GC.SuppressFinalize(this);
        }
        #endregion
    }
}