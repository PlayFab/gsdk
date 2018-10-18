
using System;
using System.Collections.Generic;

namespace Microsoft.Playfab.Gaming.GSDK.CSharp
{
    public static class GameserverSDK
    {
        // Keep an instance of the callbacks around so they don't get garbage collected
        private static ShutdownCallback shutdownCallback;
        private static HealthCallback healthCallback;
        private static MaintenanceCallback maintenanceCallback;

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
            return interop_gsdk.readyForPlayers();
        }

        /// <summary>
        /// Returns all configuration settings
        /// </summary>
        /// <returns>Optional</returns>
        public static IDictionary<string, string> getConfigSettings()
        {
            return interop_gsdk.getConfigSettings();
        }

        /// <summary>
        /// Kicks off communication threads, heartbeats, etc.
        /// Called implicitly by ReadyForPlayers if not called beforehand.
        /// </summary>
        public static void Start()
        {
            interop_gsdk.start();
        }

        /// <summary>
        /// Tells the Xcloud service information on who is connected.
        /// </summary>
        /// <param name="currentlyConnectedPlayers"></param>
        public static void UpdateConnectedPlayers(List<ConnectedPlayer> currentlyConnectedPlayers)
        {
            List<interop_ConnectedPlayer> interopPlayers = new List<interop_ConnectedPlayer>();
            foreach (var player in currentlyConnectedPlayers)
            {
                interop_ConnectedPlayer interopPlayer = new interop_ConnectedPlayer(player.PlayerId);
                interopPlayers.Add(interopPlayer);
            }
            interop_gsdk.updateConnectedPlayers(new interop_ConnectedPlayerVector(interopPlayers));
        }

        /// <summary>
        /// Gets called if the server is shutting us down
        /// </summary>
        /// <param name="callback">The callback</param>
        public static void RegisterShutdownCallback(Action callback)
        {
            shutdownCallback = new ShutdownCallback(callback);
            interop_gsdk.registerShutdownCallback(shutdownCallback);
        }

        /// <summary>
        /// Gets called when the agent needs to check on the 
        /// game's health
        /// </summary>
        /// <param name="callback">The callback</param>
        public static void RegisterHealthCallback(Func<bool> callback)
        {
            healthCallback = new HealthCallback(callback);
            interop_gsdk.registerHealthCallback(healthCallback);
        }

        /// <summary>
        /// Gets called if the server is getting a scheduled maintenance,
        /// it will get the UTC Datetime of the maintenance event as an argument.
        /// </summary>
        /// <param name="callback">The callback</param>
        public static void RegisterMaintenanceCallback(Action<DateTimeOffset> callback)
        {
            maintenanceCallback = new MaintenanceCallback(callback);
            interop_gsdk.registerMaintenanceCallback(maintenanceCallback);
        }

        /// <summary>
        /// Returns the directory whose contents will be uploaded so logs 
        /// can be easily retrieved
        /// </summary>
        /// <returns>A path to the directory to place logs in</returns>
        public static string GetLogsDirectory()
        {
            return interop_gsdk.getLogsDirectory();
        }

        /// <summary>
        /// Adds a custom log message to the GSDK log output
        /// </summary>
        /// <param name="message">The message to be logged</param>
        public static void LogMessage(string message)
        {
            interop_gsdk.logMessage(message);
        }

        // Keys for the Dictionary returned by GetConfigSettings
        public static string HeartbeatEndpointKey => interop_gsdkPINVOKE.interop_GSDK_HEARTBEAT_ENDPOINT_KEY_get();
        public static string ServerIdKey => interop_gsdkPINVOKE.interop_GSDK_SERVER_ID_KEY_get();
        public static string LogFolderKey => interop_gsdkPINVOKE.interop_GSDK_LOG_FOLDER_KEY_get();
        public static string CertificateFolderKey => interop_gsdkPINVOKE.interop_GSDK_CERTIFICATE_FOLDER_KEY_get();
        public static string TitleIdKey => interop_gsdkPINVOKE.interop_GSDK_TITLE_ID_KEY_get();
        public static string BuildIdKey => interop_gsdkPINVOKE.interop_GSDK_BUILD_ID_KEY_get();
        public static string RegionKey => interop_gsdkPINVOKE.interop_GSDK_REGION_KEY_get();

        // These two keys are only available in the map after allocation (once readyForPlayers returns true)
        public static string SessionCookieKey => interop_gsdkPINVOKE.interop_GSDK_SESSION_COOKIE_KEY_get();
        public static string SessionIdKey => interop_gsdkPINVOKE.interop_GSDK_SESSION_ID_KEY_get();

    };
}
