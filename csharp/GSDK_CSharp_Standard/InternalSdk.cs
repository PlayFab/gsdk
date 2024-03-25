using System.Collections.Concurrent;

namespace Microsoft.Playfab.Gaming.GSDK.CSharp
{
    using System;
    using System.Collections.Generic;
    using System.Globalization;
    using System.Linq;
    using System.Threading;
    using System.Threading.Tasks;
    using Model;
    using Newtonsoft.Json;

    class InternalSdk 
    {
        // Workaround to enable .Net 4.5 and netstandard1.6 (instead of using Task.CompletedTask).
        private static readonly Task CompletedTask = Task.FromResult(false);
        private GameState _state;

        private Task _heartbeatTask;
        private GSDKConfiguration _configuration;
        private IHttpClient _httpClient;
        private readonly IHttpClientFactory _httpClientFactory;
        private DateTime _cachedScheduleMaintDate;
        private readonly ManualResetEvent _signalHeartbeatEvent = new ManualResetEvent(false);
        private bool _debug;

        public ManualResetEvent TransitionToActiveEvent { get; set; }
        public IDictionary<string, string> ConfigMap { get; private set; }
        public ILogger Logger { get; private set; }

        private readonly ISystemOperations _systemOperationsWrapper;

        public GameState State
        {
            get => _state;
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

        public InternalSdk(ISystemOperations systemOperationsWrapper = null, IHttpClientFactory httpClientFactory = null)
        {
            ConnectedPlayers = new List<ConnectedPlayer>();
            InitialPlayers = new List<string>();
            TransitionToActiveEvent = new ManualResetEvent(false);
            _systemOperationsWrapper = systemOperationsWrapper ?? SystemOperations.Instance;
            _httpClientFactory = httpClientFactory ?? HttpClientFactory.Instance;
        }

        public void Start(bool debugLogs = false)
        {
            // If we already initialized everything, no need to do it again
            if (_heartbeatTask != null)
            {
                return;
            }

            _debug = debugLogs;
            this.State = GameState.Initializing;

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
            

            string heartbeatEndpoint = ConfigMap[GameserverSDK.HeartbeatEndpointKey];
            string serverId = ConfigMap[GameserverSDK.ServerIdKey];

            Logger.Log($"VM Agent Endpoint: {heartbeatEndpoint}");
            Logger.Log($"Instance Id: {serverId}");

            _httpClient = _httpClientFactory.CreateInstance($"http://{heartbeatEndpoint}/v1/sessionHosts/{serverId}");

            _signalHeartbeatEvent.Reset();
            TransitionToActiveEvent.Reset();

            _heartbeatTask = Task.Run(() => HeartbeatAsync($"http://{heartbeatEndpoint}/v1/metrics/{serverId}/gsdkinfo"));
        }

        private GSDKConfiguration GetConfiguration()
        {
            string fileName = _systemOperationsWrapper.GetEnvironmentVariableValue(GameserverSDK.GsdkConfigFileEnvVarKey);

            GSDKConfiguration localConfig;

            if (!string.IsNullOrWhiteSpace(fileName) && _systemOperationsWrapper.FileExists(fileName))
            {
                try
                {
                    localConfig = JsonConvert.DeserializeObject<GSDKConfiguration>(_systemOperationsWrapper.FileReadAllText(fileName));

                }
                catch (Exception ex)
                {
                    throw new GSDKInitializationException($"Cannot read configuration file {fileName}", ex);
                }
            }
            else
            {
                throw new GSDKInitializationException($"GSDK file - {fileName} not found");
            }

            return localConfig;
        }

        private IDictionary<string, string> CreateConfigMap(GSDKConfiguration localConfig)
        {
            var finalConfig = new ConcurrentDictionary<string, string>(StringComparer.OrdinalIgnoreCase);

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
            finalConfig[GameserverSDK.ServerIdKey] = localConfig.SessionHostId;
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

        public GameServerConnectionInfo GetGameServerConnectionInfo()
        {
            return _configuration.GameServerConnectionInfo;
        }

        private async Task HeartbeatAsync(string infoUrl)
        {
            try
            {
                await _httpClient.SendInfoAsync(infoUrl);
            }
            catch (Exception ex)
            {
                Logger.Log($"Cannot send GSDK info: {ex.Message}\r\n\r\n{ex}");
            }

            while (true)
            {
                try
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
                catch (Exception e)
                {
                    Logger.Log($"Encountered exception while sending heartbeat. Exception: {e}");
                }
            }
        }

        internal async Task SendHeartbeatAsync()
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
                HeartbeatResponse response = await _httpClient.SendHeartbeatAsync(payload);
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

                if (response.SessionConfig?.Metadata != null && response.SessionConfig.Metadata.Any())
                {
                    foreach (KeyValuePair<string, string> keyValuePair in response.SessionConfig.Metadata)
                    {
                        ConfigMap.AddIfNotNullOrEmpty(keyValuePair.Key, keyValuePair.Value);
                    }
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

            return CompletedTask;
        }
    }
}