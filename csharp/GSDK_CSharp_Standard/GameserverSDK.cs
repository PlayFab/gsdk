
using System;
using System.Collections.Generic;
using System.IO;
using System.Threading.Tasks;

namespace Microsoft.Playfab.Gaming.GSDK.CSharp
{
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
        GetManifest,
        Quarantine,
        Active,
        Terminate,
        Operation_Count
    }

    public static class GameserverSDK
    {
        // These two keys are only available after allocation (once readyForPlayers returns true)
        public const string SessionCookieKey = "sessionCookie";
        public const string SessionIdKey = "sessionId";

        private static InternalSdk _internalSdk = new InternalSdk();

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
            if (_internalSdk.State != GameState.Active)
            {
                _internalSdk.TransitionToActiveEvent.Reset();
                _internalSdk.State = GameState.StandingBy;
                _internalSdk.TransitionToActiveEvent.WaitOne();
            }

            return _internalSdk.State == GameState.Active;
        }

        /// <summary>
        /// Returns all configuration settings
        /// </summary>
        /// <returns>Optional</returns>
        public static IDictionary<string, string> getConfigSettings()
        {
            return _internalSdk.ConfigMap;
        }

        /// <summary>
        /// Kicks off communication threads, heartbeats, etc.
        /// Called implicitly by ReadyForPlayers if not called beforehand.
        /// </summary>
        public static void Start(bool debugLogs = false)
        {
            Task.WhenAll(_internalSdk.StartAsync())
                .Wait();
        }

        /// <summary>
        /// Tells the Xcloud service information on who is connected.
        /// </summary>
        /// <param name="currentlyConnectedPlayers"></param>
        public static void UpdateConnectedPlayers(IList<ConnectedPlayer> currentlyConnectedPlayers)
        {
            _internalSdk.ConnectedPlayers = currentlyConnectedPlayers;
        }

        /// <summary>
        /// Gets called if the server is shutting us down
        /// </summary>
        /// <param name="callback">The callback</param>
        public static void RegisterShutdownCallback(Action callback)
        {
            _internalSdk.ShutdownCallback = callback;
        }

        /// <summary>
        /// Gets called when the agent needs to check on the 
        /// game's health
        /// </summary>
        /// <param name="callback">The callback</param>
        public static void RegisterHealthCallback(Func<bool> callback)
        {
            _internalSdk.HealthCallback = callback;
        }

        /// <summary>
        /// Gets called if the server is getting a scheduled maintenance,
        /// it will get the UTC Datetime of the maintenance event as an argument.
        /// </summary>
        /// <param name="callback">The callback</param>
        public static void RegisterMaintenanceCallback(Action<DateTimeOffset> callback)
        {
            _internalSdk.MaintenanceCallback = callback;
        }

        /// <summary>
        /// Returns the directory whose contents will be uploaded so logs 
        /// can be easily retrieved
        /// </summary>
        /// <returns>A path to the directory to place logs in</returns>
        public static string GetLogsDirectory()
        {
            if (_internalSdk.ConfigMap.TryGetValue(InternalSdk.LogFolderKey, out string folder))
            {
                return folder;
            }

            return string.Empty;
        }

        /// <summary>
        /// Adds a custom log message to the GSDK log output
        /// </summary>
        /// <param name="message">The message to be logged</param>
        public static void LogMessage(string message)
        {
            _internalSdk.Logger.Log(message);
        }
    };
}
