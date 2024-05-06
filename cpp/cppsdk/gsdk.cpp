
// Copyright (C) Microsoft Corporation. All rights reserved.

#include "gsdkCommonPch.h"
#include "gsdkInternal.h"
#include "gsdkConfig.h"
#include "gsdkInfo.h"

namespace Microsoft
{
    namespace Azure
    {
        namespace Gaming
        {
            constexpr int c_minHeartbeatIntervalMs = 1000;
            std::unique_ptr<GSDKInternal> GSDKInternal::m_instance = nullptr;
            std::mutex GSDKInternal::m_gsdkInitMutex;
            volatile long long GSDKInternal::m_exitStatus = 0;
            std::mutex GSDKInternal::m_logLock;
            std::ofstream GSDKInternal::m_logFile;
            bool GSDKInternal::m_debug = false;
            std::unique_ptr<Configuration> GSDKInternal::testConfiguration = nullptr;

            GSDKInternal::GSDKInternal() : m_transitionToActiveEvent(), m_signalHeartbeatEvent(), m_initialPlayers()
            {
                // Need to setup the config first, as that tells us where to log
                Configuration* config = nullptr;

                // Creating the smart ptr outside the if/else so the object is still alive when we access it below
                std::unique_ptr<Configuration> configSmrtPtr = nullptr;

                // If they specified a particular config, use that, otherwise create our default
                if (testConfiguration != nullptr)
                {
                    // Using .get() instead of std::move so that the object doesn't get destroyed after this constructor
                    config = testConfiguration.get();
                }
                else
                {
                    std::string file_name = cGSDKUtils::getEnvironmentVariable("GSDK_CONFIG_FILE");
                    std::ifstream is(file_name, std::ifstream::in);

                    // If the configuration file is not there, we'll get our config from environment variables
                    if (is.fail())
                    {
                        configSmrtPtr = std::make_unique<EnvironmentVariableConfiguration>();
                    }
                    else
                    {
                        configSmrtPtr = std::make_unique<JsonFileConfiguration>(file_name);
                    }
                    config = configSmrtPtr.get();
                }

                std::unordered_map<std::string, std::string> gameCerts = config->getGameCertificates();
                for (auto it = gameCerts.begin(); it != gameCerts.end(); ++it)
                {
                    m_configSettings[it->first] = it->second;
                }

                std::unordered_map<std::string, std::string> metadata = config->getBuildMetadata();
                for (auto it = metadata.begin(); it != metadata.end(); ++it)
                {
                    m_configSettings[it->first] = it->second;
                }

                std::unordered_map<std::string, std::string> ports = config->getGamePorts();
                for (auto it = ports.begin(); it != ports.end(); ++it)
                {
                    m_configSettings[it->first] = it->second;
                }

                m_configSettings[GSDK::HEARTBEAT_ENDPOINT_KEY] = config->getHeartbeatEndpoint();
                m_configSettings[GSDK::SERVER_ID_KEY] = config->getServerId();
                m_configSettings[GSDK::LOG_FOLDER_KEY] = config->getLogFolder();
                m_configSettings[GSDK::SHARED_CONTENT_FOLDER_KEY] = config->getSharedContentFolder();
                m_configSettings[GSDK::CERTIFICATE_FOLDER_KEY] = config->getCertificateFolder();
                m_configSettings[GSDK::TITLE_ID_KEY] = config->getTitleId();
                m_configSettings[GSDK::BUILD_ID_KEY] = config->getBuildId();
                m_configSettings[GSDK::REGION_KEY] = config->getRegion();
                m_configSettings[GSDK::PUBLIC_IP_V4_ADDRESS_KEY] = config->getPublicIpV4Address();
                m_configSettings[GSDK::FULLY_QUALIFIED_DOMAIN_NAME_KEY] = config->getFullyQualifiedDomainName();

                if (m_configSettings[GSDK::HEARTBEAT_ENDPOINT_KEY].empty() || m_configSettings[GSDK::SERVER_ID_KEY].empty())
                {
                    throw GSDKInitializationException("Heartbeat endpoint and Server id are required configuration values.");
                }

                m_connectionInfo = config->getGameServerConnectionInfo();

                // We don't want to write files in our UTs
                if (config->shouldLog())
                {
                    startLog();
                }

                // Use highest frequency permitted heartbeat interval until VMAgent tells an updated one.
                m_nextHeartbeatIntervalMs = c_minHeartbeatIntervalMs;

                GSDKLogMethod method_logger(__func__);
                try
                {
                    std::string gsmsBaseUrl = m_configSettings[GSDK::HEARTBEAT_ENDPOINT_KEY];
                    std::string instanceId = m_configSettings[GSDK::SERVER_ID_KEY];

                    GSDK::logMessage("VM Agent Endpoint: " + gsmsBaseUrl);
                    GSDK::logMessage("Instance Id: " + instanceId);

                    m_heartbeatUrl.reserve(1024);
                    m_heartbeatUrl += "http://";
                    m_heartbeatUrl += gsmsBaseUrl;
                    m_heartbeatUrl += "/v1/sessionHosts/";
                    m_heartbeatUrl += instanceId;

                    m_cachedScheduledMaintenance = {};

                    curl_global_init(CURL_GLOBAL_GSDK_INIT_FLAGS);

                    m_curlHttpHeaders = nullptr;
                    m_curlHttpHeaders = curl_slist_append(m_curlHttpHeaders, "Accept: application/json");
                    m_curlHttpHeaders = curl_slist_append(m_curlHttpHeaders, "Content-Type: application/json; charset=utf-8");
                    m_curlHandle = curl_easy_init();

                    m_transitionToActiveEvent.Reset();
                    m_signalHeartbeatEvent.Reset();

                    std::string infoUrl = "http://" + gsmsBaseUrl + "/v1/metrics/" + instanceId + "/gsdkinfo";

                    // we might not want to heartbeat in our UTs
                    m_keepHeartbeatRunning = config->shouldHeartbeat();
                    m_heartbeatThread = std::thread(&GSDKInternal::heartbeatThreadFunc, this, infoUrl);
                }
                catch (const std::exception& ex)
                {
                    GSDK::logMessage(ex.what());
                    throw;
                }
            }

            GSDKInternal::~GSDKInternal()
            {
                m_keepHeartbeatRunning = false;
                m_heartbeatThread.join();
            }

			//Do not need to acquire lock for configuration becase startLog is only called from the constructor.
			//If this changes lock will be needed.
            void GSDKInternal::startLog()
            {
                if (m_logFile.is_open())
                {
                    return;
                }
                std::string logFile = "GSDK_output_" + std::to_string((unsigned long long)time(nullptr)) + ".txt";
                std::string logFolder = m_configSettings[GSDK::LOG_FOLDER_KEY];
                if (!logFolder.empty() && !cGSDKUtils::createDirectoryIfNotExists(logFolder)) // If we couldn't successfully create the path, just use the current directory
                {
                    logFolder = "";
                }
#if _WIN32
                if (!logFolder.empty() && logFolder.back() != '/' && logFolder.back() != '\\')
                {
                    logFolder.append("\\");
                }
#else
                if (!logFolder.empty() && logFolder.back() != '/')
                {
                    logFolder.append("/");
                }
#endif
                std::string logPath = logFolder + logFile;
                m_logFile.open(logPath.c_str(), std::ofstream::out);
            }

            void GSDKInternal::heartbeatThreadFunc(std::string infoUrl)
            {
                resetCurl();
                curl_easy_setopt(m_curlHandle, CURLOPT_URL, infoUrl.c_str());

                m_receivedData = "";
                curl_easy_setopt(m_curlHandle, CURLOPT_CUSTOMREQUEST, "POST");

                Json::Value jsonInfoRequest;
                jsonInfoRequest[GSDK_INFO_FLAVOR_KEY] = GSDK_INFO_FLAVOR;
                jsonInfoRequest[GSDK_INFO_VERSION_KEY] = GSDK_INFO_VERSION;

                std::string infoRequest = jsonInfoRequest.toStyledString();
                curl_easy_setopt(m_curlHandle, CURLOPT_POSTFIELDS, infoRequest.c_str());
                curl_easy_perform(m_curlHandle);

                long http_code = 0;
                curl_easy_getinfo(m_curlHandle, CURLINFO_RESPONSE_CODE, &http_code);
                if (http_code >= 300)
                {
                    GSDK::logMessage("Received non-success code from Agent when sending GSDK info.  Status Code: " + std::to_string(http_code) + " Response Body: " + m_receivedData);
                }

                while (m_keepHeartbeatRunning)
                {
                    if (m_signalHeartbeatEvent.Wait(m_nextHeartbeatIntervalMs))
                    {
                        if (m_debug) GSDK::logMessage("State transition signaled an early heartbeat.");
                        m_signalHeartbeatEvent.Reset(); // We've handled this signal, so reset the event
                    }

                    sendHeartbeat();
                    receiveHeartbeatResponse();
                }
            }

            size_t GSDKInternal::curlReceiveData(char* buffer, size_t blockSize, size_t blockCount, void*)
            {
                std::lock_guard<std::mutex> lock(get().m_receivedDataMutex);

                get().m_receivedData.append(buffer, blockSize * blockCount);
                return (blockSize * blockCount);
            }

            void GSDKInternal::resetCurl()
            {
                curl_easy_reset(m_curlHandle);
                curl_easy_setopt(m_curlHandle, CURLOPT_URL, m_heartbeatUrl.c_str());
                curl_easy_setopt(m_curlHandle, CURLOPT_HTTPHEADER, m_curlHttpHeaders);
                curl_easy_setopt(m_curlHandle, CURLOPT_WRITEFUNCTION, curlReceiveData);
            }

            void GSDKInternal::sendHeartbeat()
            {
                resetCurl();
                m_receivedData = "";
                curl_easy_setopt(m_curlHandle, CURLOPT_CUSTOMREQUEST, "PATCH");
                std::string request = encodeHeartbeatRequest();
                curl_easy_setopt(m_curlHandle, CURLOPT_POSTFIELDS, request.c_str());
                curl_easy_perform(m_curlHandle);
            }

            std::string GSDKInternal::encodeHeartbeatRequest()
            {
                Json::Value jsonHeartbeatRequest;

                jsonHeartbeatRequest["CurrentGameState"] = GameStateNames[static_cast<int>(m_heartbeatRequest.m_currentGameState)];

                auto healthCallback = m_healthCallback;
                if (healthCallback != nullptr)
                {
                    m_heartbeatRequest.m_isGameHealthy = healthCallback();
                }
                jsonHeartbeatRequest["CurrentGameHealth"] = m_heartbeatRequest.m_isGameHealthy ? "Healthy" : "Unhealthy";

                Json::Value jsonConnectedPlayerInfo;
                for (ConnectedPlayer connectedPlayer : m_heartbeatRequest.m_connectedPlayers)
                {
                    Json::Value playerInfo;
                    playerInfo["PlayerId"] = connectedPlayer.m_playerId;
                    jsonConnectedPlayerInfo.append(playerInfo);
                }
                jsonHeartbeatRequest["CurrentPlayers"] = jsonConnectedPlayerInfo;

                return jsonHeartbeatRequest.toStyledString();
            }

            std::tm GSDKInternal::parseDate(const std::string& dateStr) // note: this code only supports ISO 8601 UTC date-times in the format yyyy-mm-ddThh:mm:ssZ
            {
                std::tm ret = {};
                bool failed;

                try
                {
                    std::istringstream iss(dateStr);
                    iss >> std::get_time(&ret, "%Y-%m-%dT%T");
                    failed = iss.fail();
                }
                catch (...)
                {
                    failed = true;
                }

                if (failed)
                {
                    ret = {};
                    ret.tm_year = 100;
                }

                return ret;
            }

            void GSDKInternal::setState(GameState state)
            {
                std::lock_guard<std::mutex> lock(m_stateMutex);

                if (m_heartbeatRequest.m_currentGameState != state)
                {
                    m_heartbeatRequest.m_currentGameState = state;
                    m_signalHeartbeatEvent.Signal();
                }
            }

            void GSDKInternal::setConnectedPlayers(const std::vector<ConnectedPlayer>& currentConnectedPlayers)
            {
                std::lock_guard<std::mutex> lock(m_playersMutex);
                m_heartbeatRequest.m_connectedPlayers = currentConnectedPlayers;
            }

            void GSDKInternal::runShutdownCallback()
            {
                std::function<void()> shutdownCallback = get().m_shutdownCallback;
                if (shutdownCallback != nullptr)
                {
                    shutdownCallback();
                }
                get().m_keepHeartbeatRunning = false;
            }

            void GSDKInternal::decodeHeartbeatResponse(const std::string& responseJson)
            {
                Json::CharReaderBuilder jsonReaderFactory;
                std::unique_ptr<Json::CharReader> jsonReader(jsonReaderFactory.newCharReader());
                Json::Value heartbeatResponse;
                JSONCPP_STRING jsonParseErrors;
                bool parsedSuccessfully = jsonReader->parse(responseJson.c_str(), responseJson.c_str() + responseJson.length(), &heartbeatResponse, &jsonParseErrors);

                if (!parsedSuccessfully) {
                    GSDK::logMessage("Failed to parse heartbeat");
                    GSDK::logMessage(jsonParseErrors);
                    GSDK::logMessage("Message: " + responseJson);
                    return;
                }

                try {
                    if (heartbeatResponse.isMember("sessionConfig"))
                    {
						std::lock_guard<std::mutex> lock(m_configMutex);
						Json::Value sessionConfig = heartbeatResponse["sessionConfig"];
                        for (Json::ValueIterator i = sessionConfig.begin(); i != sessionConfig.end(); ++i)
                        {
                            if ((*i).isString())
                            {
								m_configSettings[i.key().asCString()] = (*i).asCString();
                            }
                        }

                        // Update initial players only if this is the first time populating it.
                        if (m_initialPlayers.empty() && sessionConfig.isMember("initialPlayers"))
                        {
                            Json::Value players = sessionConfig["initialPlayers"];

                            for (Json::ArrayIndex i = 0; i < players.size(); ++i)
                            {
                                m_initialPlayers.push_back(players[i].asCString());
                            }
                        }

                        if (sessionConfig.isMember("metadata"))
                        {
                            Json::Value sessionMetadata = sessionConfig["metadata"];
                            for (Json::ValueIterator i = sessionMetadata.begin(); i != sessionMetadata.end(); ++i)
                            {
                                if ((*i).isString())
                                {
									m_configSettings[i.key().asCString()] = (*i).asCString();
                                }
                            }
                        }
                    }

                    if (heartbeatResponse.isMember("nextScheduledMaintenanceUtc"))
                    {
                        tm nextMaintenance = parseDate(heartbeatResponse["nextScheduledMaintenanceUtc"].asCString());
                        time_t nextMaintenanceTime = cGSDKUtils::tm2timet_utc(&nextMaintenance);
                        time_t cachedMaintenanceTime = cGSDKUtils::tm2timet_utc(&m_cachedScheduledMaintenance);
                        double diff = difftime(nextMaintenanceTime, cachedMaintenanceTime);
                        auto maintCallback = m_maintenanceCallback;

                        // If the cached time converted to -1, it means we haven't cached anything yet
                        if (maintCallback != nullptr && (static_cast<int>(diff) != 0 || cachedMaintenanceTime == -1))
                        {
                            maintCallback(nextMaintenance);
                            m_cachedScheduledMaintenance = nextMaintenance; // cache it so we only notify once
                        }
                    }

                    if (heartbeatResponse.isMember("maintenanceSchedule"))
                    {
                        auto maintV2Callback = m_maintenanceV2Callback;

                        Json::Value scheduleJson = heartbeatResponse["maintenanceSchedule"];
                        MaintenanceSchedule schedule{};
                        schedule.m_documentIncarnation = scheduleJson["documentIncarnation"].asString();

                        for (const auto& eventJson : scheduleJson["Events"])
                        {
                            MaintenanceEvent eventData{};
                            eventData.m_eventId = eventJson["eventId"].asString();
                            eventData.m_eventType = eventJson["eventType"].asString();
                            eventData.m_resourceType = eventJson["resourceType"].asString();
                            std::vector<std::string> resources{};
                            for (const auto& resource : eventJson["Resources"])
                            {
                                resources.push_back(resource.asString());
                            }
                            eventData.m_resources = resources;
                            eventData.m_eventStatus = eventJson["eventStatus"].asString();
                            eventData.m_notBefore = parseDate(eventJson["notBefore"].asCString());
                            eventData.m_description = eventJson["description"].asString();
                            eventData.m_eventSource = eventJson["eventSource"].asString();
                            eventData.m_durationInSeconds = eventJson["durationInSeconds"].asInt();

                            schedule.m_events.push_back(eventData);
                        }

                        if (maintV2Callback != nullptr)
                        {
                            maintV2Callback(schedule);
                        }
                    }

                    if (heartbeatResponse.isMember("operation"))
                    {
                        try
                        {
                            if (m_debug) {
                                GSDK::logMessage("Heartbeat request: { state = " + std::string(GameStateNames[static_cast<int>(m_heartbeatRequest.m_currentGameState)]) + "}"
                                    + " response: { operation = " + heartbeatResponse["operation"].asString() + "}");
                            }

                            Operation nextOperation = OperationMap.at(heartbeatResponse["operation"].asCString());

                            switch (nextOperation)
                            {
                            case Operation::Continue:
                                // No action required
                                break;
                            case Operation::Active:
                                if (m_heartbeatRequest.m_currentGameState != GameState::Active)
                                {
                                    setState(GameState::Active);
                                    m_transitionToActiveEvent.Signal();
                                }
                                break;
                            case Operation::Terminate:
                                if (m_heartbeatRequest.m_currentGameState != GameState::Terminating)
                                {
                                    setState(GameState::Terminating);
                                    m_transitionToActiveEvent.Signal();
                                    m_shutdownThread = std::async(std::launch::async, &runShutdownCallback);
                                }
                                break;
                            default:
                                GSDK::logMessage("Unhandled operation received: " + std::string(OperationNames[static_cast<int>(nextOperation)]));
                            }
                        }
                        catch (std::out_of_range&)
                        {
                            GSDK::logMessage("Unknown operation received: " + heartbeatResponse["operation"].asString());
                        }
                    }

                    if (heartbeatResponse.isMember("nextHeartbeatIntervalMs"))
                    {
                        m_nextHeartbeatIntervalMs = heartbeatResponse["nextHeartbeatIntervalMs"].asInt();

                        // Clamp to the minimum permitted interval.
                        if (m_nextHeartbeatIntervalMs < c_minHeartbeatIntervalMs)
                        {
                            m_nextHeartbeatIntervalMs = c_minHeartbeatIntervalMs;
                        }
                    }
                    else
                    {
                        // If VMagent didn't specify a heartbeat interval, default to higher frequency for safety.
                        m_nextHeartbeatIntervalMs = c_minHeartbeatIntervalMs;
                    }
                }
                catch (Json::Exception& ex) {
                    // we caught an exception - log it out
                    GSDK::logMessage("An error occured while processing heartbeat.");
                    GSDK::logMessage(ex.what());
                    GSDK::logMessage("Message: " + responseJson);
                }
            }

            void GSDKInternal::receiveHeartbeatResponse()
            {
                long http_code = 0;
                curl_easy_getinfo(m_curlHandle, CURLINFO_RESPONSE_CODE, &http_code);
                if (http_code >= 300)
                {
                    GSDK::logMessage("Received non-success code from Agent.  Status Code: " + std::to_string(http_code) + " Response Body: " + m_receivedData);
                    return;
                }

                decodeHeartbeatResponse(m_receivedData);
            }

            Microsoft::Azure::Gaming::GSDKInternal& GSDKInternal::get()
            {
                std::unique_lock<std::mutex> lock(m_gsdkInitMutex);

                if (!m_instance)
                {
                    m_instance = std::make_unique<GSDKInternal>();
                }
                return *m_instance;
            }

            void GSDK::start(bool debugLogs)
            {
                GSDKInternal::m_debug = debugLogs;
                GSDKInternal::get();
            }

            bool GSDK::readyForPlayers()
            {
                if (GSDKInternal::get().m_heartbeatRequest.m_currentGameState != GameState::Active)
                {
                    GSDKInternal::get().setState(GameState::StandingBy);
                    GSDKInternal::get().m_transitionToActiveEvent.Wait();
                }

                return GSDKInternal::get().m_heartbeatRequest.m_currentGameState == GameState::Active;
            }

            const Microsoft::Azure::Gaming::GameServerConnectionInfo &GSDK::getGameServerConnectionInfo()
            {
                return GSDKInternal::get().m_connectionInfo;
            }

            const std::unordered_map<std::string, std::string> GSDK::getConfigSettings()
            {
				std::lock_guard<std::mutex> lock(GSDKInternal::get().m_configMutex);
                return GSDKInternal::get().m_configSettings;
            }

            void GSDK::updateConnectedPlayers(const std::vector<ConnectedPlayer>& currentlyConnectedPlayers)
            {
                GSDKInternal::get().setConnectedPlayers(currentlyConnectedPlayers);
            }

            void GSDK::registerShutdownCallback(std::function< void() > callback)
            {
                GSDKInternal::get().m_shutdownCallback = callback;
            }

            void GSDK::registerHealthCallback(std::function< bool() > callback)
            {
                GSDKInternal::get().m_healthCallback = callback;
            }

            void GSDK::registerMaintenanceCallback(std::function< void(const tm&) > callback)
            {
                GSDKInternal::get().m_maintenanceCallback = callback;
            }

            void GSDK::registerMaintenanceV2Callback(std::function<void(const MaintenanceSchedule&)> callback)
            {
                GSDKInternal::get().m_maintenanceV2Callback = callback;
            }

            unsigned int GSDK::logMessage(const std::string& message)
            {
                std::unique_lock<std::mutex> lock(GSDKInternal::m_logLock);
                GSDKInternal::m_logFile << message.c_str() << std::endl;
                GSDKInternal::m_logFile.flush();
                return 0;
            }

            const std::string GSDK::getLogsDirectory()
            {
                // Declare as static so that it doesn't live on the stack (since we're returning a reference)
                static const std::string empty = "";

				std::lock_guard<std::mutex> lock(GSDKInternal::get().m_configMutex);

                const std::unordered_map<std::string, std::string> config = GSDKInternal::get().m_configSettings;
                auto it = config.find(GSDK::LOG_FOLDER_KEY);

                if (it == config.end())
                {
                    return empty;
                }
                else
                {
					return it->second;
                }
            }

            const std::string GSDK::getSharedContentDirectory()
            {
                // Declare as static so that it doesn't live on the stack (since we're returning a reference)
                static const std::string empty = "";

				std::lock_guard<std::mutex> lock(GSDKInternal::get().m_configMutex);

				const std::unordered_map<std::string, std::string> config = GSDKInternal::get().m_configSettings;

                auto it = config.find(GSDK::SHARED_CONTENT_FOLDER_KEY);

                if (it == config.end())
                {
                    return empty;
                }
                else
                {
                    return it->second;
                }
            }

            const std::vector<std::string>& GSDK::getInitialPlayers()
            {
                return GSDKInternal::get().m_initialPlayers;
            }
        }
    }
}
