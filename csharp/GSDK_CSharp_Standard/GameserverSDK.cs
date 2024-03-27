namespace Microsoft.Playfab.Gaming.GSDK.CSharp
{
    using Microsoft.Playfab.Gaming.GSDK.CSharp.Model;
    using System;
    using System.Collections.Generic;
    
    public enum GameState
    {
        Invalid,
        Initializing,
        StandingBy,
        Active,
        Terminating,
        Terminated,
        Quarentined
    }

    public enum GameOperation
    {
        Invalid,
        Continue,
        Active,
        Terminate
    }

    public static class GameserverSDK
    {
        // These two keys are only available after allocation (once readyForPlayers returns true)
        public const string SessionCookieKey = "sessionCookie";
        public const string SessionIdKey = "sessionId";
        public const string HeartbeatEndpointKey = "heartbeatEndpoint";
        public const string ServerIdKey = "serverId";
        public const string LogFolderKey = "logFolder";
        public const string SharedContentFolderKey = "sharedContentFolder";
        public const string CertificateFolderKey = "certificateFolder";
        public const string TitleIdKey = "titleId";
        public const string BuildIdKey = "buildId";
        public const string RegionKey = "region";
        public const string VmIdKey = "vmId";
        public const string PublicIpV4AddressKey = "publicIpV4Address";
        public const string FullyQualifiedDomainNameKey = "fullyQualifiedDomainName";

        public const string GsdkConfigFileEnvVarKey = "GSDK_CONFIG_FILE";

        private static readonly InternalSdk _internalSdk = new InternalSdk();

        /// <summary>
        /// Called when the game server is ready to accept clients.
        /// If Start() hasn't been called by this point, it will be
        /// called implicitly here.
        /// </summary>
        /// <remarks>Required</remarks>
        /// <returns>
        /// True if the game server was allocated (clients are about to connect).
        /// False if the game server was terminated and is about to be shut down.
        /// </returns>
        public static bool ReadyForPlayers()
        {
            _internalSdk.Start();

            if (_internalSdk.State != GameState.Active)
            {
                _internalSdk.TransitionToActiveEvent.Reset();
                _internalSdk.State = GameState.StandingBy;
                _internalSdk.TransitionToActiveEvent.WaitOne();
            }

            return _internalSdk.State == GameState.Active;
        }

        /// <summary>
        /// Gets information (ipAddress and ports) for connecting to the game server, as well as the ports the
        /// game server should listen on.
        /// </summary>
        /// <returns></returns>
        public static GameServerConnectionInfo GetGameServerConnectionInfo()
        {
            _internalSdk.Start();
            return _internalSdk.GetGameServerConnectionInfo();
        }

        /// <summary>
        /// Returns all configuration settings
        /// </summary>
        /// <returns>Optional</returns>
        public static IDictionary<string, string> getConfigSettings()
        {
            _internalSdk.Start();

            return new Dictionary<string, string>(_internalSdk.ConfigMap, StringComparer.OrdinalIgnoreCase);
        }

        /// <summary>
        /// Kicks off communication threads, heartbeats, etc.
        /// Called implicitly by ReadyForPlayers if not called beforehand.
        /// </summary>
        public static void Start(bool debugLogs = false)
        {
            _internalSdk.Start(debugLogs);
        }

        /// <summary>
        /// Tells the PlayFab service information on who is connected.
        /// </summary>
        /// <param name="currentlyConnectedPlayers"></param>
        public static void UpdateConnectedPlayers(IList<ConnectedPlayer> currentlyConnectedPlayers)
        {
            _internalSdk.Start();
            _internalSdk.ConnectedPlayers = currentlyConnectedPlayers;
        }

        /// <summary>
        /// Gets called if the server is shutting us down
        /// </summary>
        /// <param name="callback">The callback</param>
        public static void RegisterShutdownCallback(Action callback)
        {
            _internalSdk.Start();

            _internalSdk.ShutdownCallback = callback;
        }

        /// <summary>
        /// Gets called when the agent needs to check on the
        /// game's health
        /// </summary>
        /// <param name="callback">The callback</param>
        public static void RegisterHealthCallback(Func<bool> callback)
        {
            _internalSdk.Start();

            _internalSdk.HealthCallback = callback;
        }

        /// <summary>
        /// DEPRECATED Use RegisterMaintenanceV2Callback instead.
        /// Gets called if the server is getting a scheduled maintenance,
        /// it will get the UTC Datetime of the maintenance event as an argument.
        /// </summary>
        /// <param name="callback">The callback</param>
        [Obsolete]
        public static void RegisterMaintenanceCallback(Action<DateTimeOffset> callback)
        {
            _internalSdk.Start();

            _internalSdk.MaintenanceCallback = callback;
        }

        /// <summary>
        /// Gets called if the server is getting a scheduled maintenance,
        /// it will get the maintenance event as an argument.
        /// </summary>
        /// <remarks>
        /// https://learn.microsoft.com/azure/virtual-machines/windows/scheduled-events#event-properties
        /// </remarks>
        /// <param name="callback">The callback</param>
        public static void RegisterMaintenanceV2Callback(Action<MaintenanceSchedule> callback)
        {
            _internalSdk.Start();

            _internalSdk.MaintenanceV2Callback = callback;
        }

        /// <summary>
        /// Returns the directory whose contents will be uploaded so logs
        /// can be easily retrieved
        /// </summary>
        /// <returns>A path to the directory to place logs in</returns>
        public static string GetLogsDirectory()
        {
            _internalSdk.Start();

            if (_internalSdk.ConfigMap.TryGetValue(LogFolderKey, out string folder))
            {
                return folder;
            }

            return string.Empty;
        }

        /// <summary>
        /// Returns the directory whose contents are shared among all game servers within a VM.
        /// </summary>
        /// <returns>A path to the directory where shared content can be stored (temporarily). </returns>
        public static string GetSharedContentDirectory()
        {
            _internalSdk.Start();

            return _internalSdk.ConfigMap.TryGetValue(SharedContentFolderKey, out string folder) ? folder : string.Empty;
        }

        /// <summary>
        /// After allocation, returns a list of the initial players that have access to this game server, used by PlayFab's
        /// Matchmaking offering
        /// </summary>
        /// <returns>A list of player ids of the initial players that will connect</returns>
        public static IList<string> GetInitialPlayers()
        {
            _internalSdk.Start();
        
            return new List<string>(_internalSdk.InitialPlayers);
        }

        /// <summary>
        /// Adds a custom log message to the GSDK log output
        /// </summary>
        /// <param name="message">The message to be logged</param>
        public static void LogMessage(string message)
        {
            _internalSdk.Start();
            _internalSdk.Logger.Log(message);
        }
    }
}