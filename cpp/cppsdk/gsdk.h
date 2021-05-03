
// Copyright (C) Microsoft Corporation. All rights reserved.

#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include <exception>
#include <vector>
#include <stdexcept>

namespace Microsoft
{
    namespace Azure
    {
        namespace Gaming
        {
            class ConnectedPlayer // need to link this to the C# version
            {
            public:
                std::string m_playerId;

                ConnectedPlayer(std::string playerId)
                {
                    m_playerId = playerId;
                }
            };

            /// <summary>
            /// A class that captures details about a game server port.
            /// </summary>
            class GamePort
            {
                public:
                    /// <summary>
                    /// The friendly name / identifier for the port, specified by the game developer in the Build configuration.
                    /// </summary>
                    std::string m_name;

                    /// <summary>
                    /// The port at which the game server should listen on (maps externally to <see cref="m_clientConnectionPort" />).
                    /// For process based servers, this is determined by Control Plane, based on the ports available on the VM.
                    /// For containers, this is specified by the game developer in the Build configuration.
                    /// </summary>
                    int m_serverListeningPort;

                    /// <summary>
                    /// The public port to which clients should connect (maps internally to <see cref="m_serverListeningPort" />).
                    /// </summary>
                    int m_clientConnectionPort;

                    GamePort() {}

                    GamePort(std::string name, int serverListeningPort, int clientConnectionPort)
                    {
                        m_name = name;
                        m_serverListeningPort = serverListeningPort;
                        m_clientConnectionPort = clientConnectionPort;
                    }
            };

            class GameServerConnectionInfo
            {
                public:
                    std::string m_publicIpV4Address;
                    std::vector<GamePort> m_gamePortsConfiguration;

                    GameServerConnectionInfo() {}

                    GameServerConnectionInfo(std::string publicIpV4Address, std::vector<GamePort> gamePortsConfiguration)
                    {
                        m_publicIpV4Address = publicIpV4Address;
                        m_gamePortsConfiguration = gamePortsConfiguration;
                    }
            };

            class GSDKInitializationException : public std::runtime_error
            {
                using std::runtime_error::runtime_error;
            };

            class GSDK
            {
            public:
                /// <summary>Renamed from WaitForSessionAssignment. Called when the game server is ready to accept clients.  If Start() hasn't been called by this point, it will be called implicitly here.</summary>
                /// <remarks>Required. This is a blocking call and will only return when this server is either allocated (a player is about to connect) or terminated.</remarks>
                /// <returns>True if the server is allocated (will receive players shortly). False if the server is terminated. </returns>
                static bool readyForPlayers();

                /// <summary>
                /// Gets information (ipAddress and ports) for connecting to the game server, as well as the ports the
                /// game server should listen on.
                /// </summary>
                /// <returns></returns>
                static const GameServerConnectionInfo &getGameServerConnectionInfo();

                /// <summary>Returns all configuration settings</summary>
                /// <returns>unordered map of string key:value configuration setting values</returns>
                static const std::unordered_map<std::string, std::string> getConfigSettings();

                /// <summary>Kicks off communication threads, heartbeats, etc.  Called implicitly by ReadyForPlayers if not called beforehand.</summary>
                /// <param name="debugLogs">Enables outputting additional logs to the GSDK log file.</param>
                static void start(bool debugLogs = false);

                /// <summary>Tells the Xcloud service information on who is connected.</summary>
                /// <param name="currentlyConnectedPlayers"></param>
                static void updateConnectedPlayers(const std::vector<ConnectedPlayer> &currentlyConnectedPlayers);

                /// <summary>Gets called if the server is shutting us down</summary>
                static void registerShutdownCallback(std::function<void()> callback);

                /// <summary>Gets called when the agent needs to check on the game's health</summary>
                static void registerHealthCallback(std::function<bool()> callback);

                /// <summary>Gets called if the server is getting a scheduled maintenance, it will get the UTC time of the maintenance event as an argument.</summary>
                static void registerMaintenanceCallback(std::function<void(const tm &)> callback);

                /// <summary>outputs a message to the log</summary>
                static unsigned int logMessage(const std::string &message);

                /// <summary>Returns a path to the directory where logs will be mapped to the VM host</summary>
                static const std::string getLogsDirectory();

                /// <summary>Returns a path to the directory shared by all game servers to cache data.</summary>
                static const std::string getSharedContentDirectory();

                /// <summary>After allocation, returns a list of the initial players that have access to this game server, used by PlayFab's Matchmaking offering</summary>
                static const std::vector<std::string> &getInitialPlayers();

                // Keys for the map returned by getConfigSettings

                static constexpr const char* HEARTBEAT_ENDPOINT_KEY = "gsmsBaseUrl";
                static constexpr const char* SERVER_ID_KEY = "instanceId";
                static constexpr const char* LOG_FOLDER_KEY = "logFolder";
                static constexpr const char* SHARED_CONTENT_FOLDER_KEY = "sharedContentFolder";
                static constexpr const char* CERTIFICATE_FOLDER_KEY = "certificateFolder";
                static constexpr const char* TITLE_ID_KEY = "titleId";
                static constexpr const char* BUILD_ID_KEY = "buildId";
                static constexpr const char* REGION_KEY = "region";
                static constexpr const char* VM_ID_KEY = "vmId";
                static constexpr const char* PUBLIC_IP_V4_ADDRESS_KEY = "publicIpV4Address";
                static constexpr const char* FULLY_QUALIFIED_DOMAIN_NAME_KEY = "fullyQualifiedDomainName";

                // These two keys are only available after allocation (once readyForPlayers returns true)
                static constexpr const char* SESSION_COOKIE_KEY = "sessionCookie";
                static constexpr const char* SESSION_ID_KEY = "sessionId";
            };
        }
    }
}
