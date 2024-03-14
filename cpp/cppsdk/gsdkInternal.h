
// Copyright (C) Microsoft Corporation. All rights reserved.

#pragma once

#include "gsdk.h"
#include "gsdkLog.h"
#include "gsdkUtils.h"
#include "ManualResetEvent.h"
#include "gsdkConfig.h"

namespace Microsoft
{
    namespace Azure
    {
        namespace Gaming
        {
            #define MAKE_ENUM(VAR) VAR,
            #define MAKE_STRINGS(VAR) #VAR,

            #define GAME_STATES(DO) \
                DO( Invalid ) \
                DO( Initializing ) \
                DO( StandingBy ) \
                DO( Active ) \
                DO( Terminating ) \
                DO( Terminated ) \
                DO( Quarantined ) \

            enum class GameState // send to Agent
            {
                GAME_STATES(MAKE_ENUM)
            };

            const char* const GameStateNames[] =
            {
                GAME_STATES(MAKE_STRINGS)
            };

            #define GAME_OPERATIONS(DO) \
                DO( Invalid ) \
                DO( Continue ) \
                DO( GetManifest ) \
                DO( Quarantine ) \
                DO( Active ) \
                DO( Terminate ) \
                DO( Operation_Count ) \

            enum class Operation // get back from Agent
            {
                GAME_OPERATIONS(MAKE_ENUM)
            };

            const char* const OperationNames[] =
            {
                GAME_OPERATIONS(MAKE_STRINGS)
            };


            class SessionConfig
            {
            public:
                std::string m_sessionId;
                std::string m_sessionCookie;

                std::map<std::string, std::string> toMap()
                {
                    std::map<std::string, std::string> ret;
                    ret["sessionId"] = m_sessionId;
                    ret["sessionCookie"] = m_sessionCookie;
                    return ret;
                }
            };


            class SessionHostHeartbeatInfo
            {
            public:
                std::string currentGameState; // The current game state. For example - StandingBy, Active, etc.
                int currentGameHealth; // The last queried game host health status
                std::vector<ConnectedPlayer> currentPlayers; // Keeps track of the current list of connected players
                int nextHeartbeatIntervalMs; // The number of milliseconds to wait before sending the next heartbeat.

                Operation operation; //The next operation the VM Agent wants us to take
                SessionConfig sessionConfig; // The configuration sent down to the game host from Control Plane
                time_t nextScheduledMaintenanceUtc; // The next scheduled maintenance time from Azure, in UTC
            };


            struct HeartbeatRequest
            {
                HeartbeatRequest()
                {
                    m_currentGameState = GameState::Initializing;
                    m_isGameHealthy = true;
                }

                volatile GameState m_currentGameState;
                bool m_isGameHealthy;
                std::vector<ConnectedPlayer> m_connectedPlayers;
            };


            struct HeartbeatResponse
            {
                HeartbeatResponse()
                {
                    m_errorValue = 0;
                    m_nextScheduledMaintenanceUtc = 0; // set to POSIX start time
                }

                std::string m_operation;
                unsigned int m_nextHeartbeatMilliseconds;
                unsigned short m_statusCode;
                int m_errorValue;
                std::string m_errorMessage;
                std::string m_serverState;
                time_t m_nextScheduledMaintenanceUtc;
            };


            class GSDKInternal
            {
                friend class GSDK;
                friend class GSDKTests;
            public:
                // These must be public for unique_ptr to work
                GSDKInternal();
                ~GSDKInternal();

            private:
                // NOTE: Making this map non-static, because otherwise the heartbeat thread
                // will throw an access violation exception when the game server main loop returns
                // (As C++ starts getting rid of all statics)
                #define MAKE_OPERATION_MAP(VAR) {#VAR, Operation::VAR},
                const std::map<std::string, Operation> OperationMap =
                {
                    GAME_OPERATIONS(MAKE_OPERATION_MAP)
                };

                std::string m_agentEntpoint;
                HeartbeatRequest m_heartbeatRequest;
                std::string m_sessionCookie;
                int m_heatbeatInterval;
                std::string m_heartbeatUrl;

                std::function<void()> m_shutdownCallback;
                std::function<bool()> m_healthCallback;
                std::function<void(const tm &)> m_maintenanceCallback;

                GameServerConnectionInfo m_connectionInfo;
                std::unordered_map<std::string, std::string> m_configSettings;
                tm m_cachedScheduledMaintenance;

                std::atomic<bool> m_keepHeartbeatRunning;
                
                // NOTE: DO NOT make this a std::future instead of a std::thread.
                // 
                // Something about how the CRT runtime cleans things up makes it so that
                // if this is a std::future (from a std::async), when we exit a game program
                // from c#, by the time it reaches the c++ gsdk destructor, the heartbeat thread
                // is gone, and calling wait on the std::future will hang forever. However, calling
                // join on the std::thread doesn't hang, it seems to understand the thread exited.
                std::thread m_heartbeatThread;

                std::future<void> m_shutdownThread;

                CURL *m_curlHandle; // only valid for heartbeat thread
                curl_slist *m_curlHttpHeaders; // only valid for heartbeat thread
                std::mutex m_receivedDataMutex;
                std::string m_receivedData;
                ManualResetEvent m_transitionToActiveEvent;
                ManualResetEvent m_signalHeartbeatEvent;
                std::mutex m_stateMutex;
                std::mutex m_playersMutex;

                std::vector<std::string> m_initialPlayers;

                static std::unique_ptr<GSDKInternal> m_instance;
                static std::mutex m_gsdkInitMutex;

                static volatile long long m_exitStatus;
                static std::mutex m_logLock;
                static std::ofstream m_logFile;

                void start(std::string infoUrl);
                void heartbeatThreadFunc();
                static size_t curlReceiveData(char *buffer, size_t blockSize, size_t blockCount, void *);
                static void runShutdownCallback();
                
                static bool m_debug;

                void startLog();
                void resetCurl();
                void sendHeartbeat();
                void receiveHeartbeatResponse();

                // These two methods are used for unit testing as well as regular operation.
                std::string encodeHeartbeatRequest();
                void decodeHeartbeatResponse(const std::string &responseJson);
				std::mutex m_configMutex;
                int m_nextHeartbeatIntervalMs;

                std::tm parseDate(const std::string &dateStr);
                void setState(GameState state);
                void setConnectedPlayers(const std::vector<ConnectedPlayer> &currentConnectedPlayers);

                static GSDKInternal &get();
                static std::unique_ptr<Configuration> testConfiguration; // may be overriden by unit tests
            };

        }
    }
}
