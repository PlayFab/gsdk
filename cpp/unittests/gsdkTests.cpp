#include "stdafx.h"
#include "CppUnitTest.h"

#include "..\cppsdk\gsdk.h"
#include "..\cppsdk\gsdkInternal.h"

#include "TestConfig.h"

#include <chrono>
#include <thread>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Microsoft
{
    namespace Azure
    {
        namespace Gaming
        {
            TEST_CLASS(GSDKTests)
            {
            public:
                TEST_METHOD_CLEANUP(SetTestInstanceToNull)
                {
                    // Cleaning up the test instance between tests
                    GSDKInternal::testConfiguration.reset();
                    GSDKInternal::m_instance.reset();
                }

                TEST_METHOD(ConfigAllSetInitializesFine)
                {
                    std::string heartbeatEndpoint = "testEndpoint";
                    std::string serverId = "testServerId";
                    std::string logFolder = "testLogFolder";
                    std::string sharedContentFolder = "testSharedContentFolder";
                    std::string certFolder = "testCertFolder";
                    std::string titleId = "testTitleId";
                    std::string buildId = "testBuildId";
                    std::string region = "testRegion";
                    std::unordered_map<std::string, std::string> gameCerts = std::unordered_map<std::string, std::string>();
                    gameCerts["cert1"] = "thumbprint1";
                    gameCerts["cert2"] = "thumbprint2";
                    std::unordered_map<std::string, std::string> metadata = std::unordered_map<std::string, std::string>();
                    metadata["key1"] = "value1";
                    metadata["key2"] = "value2";
                    std::unordered_map<std::string, std::string> ports = std::unordered_map<std::string, std::string>();
                    ports["port1"] = "1111";
                    ports["port2"] = "2222";
                    GSDKInternal::testConfiguration = std::make_unique<TestConfig>(heartbeatEndpoint, serverId, logFolder, sharedContentFolder, certFolder, gameCerts, titleId, buildId, region, metadata, ports);
                    GSDK::start();
                    const std::unordered_map<std::string, std::string> &config = GSDK::getConfigSettings();
                    Assert::AreEqual(heartbeatEndpoint, config.at("gsmsBaseUrl"), L"Ensuring heartbeat endpoint was set.");
                    Assert::AreEqual(serverId, config.at("instanceId"), L"Ensuring server id was set.");
                    Assert::AreEqual(logFolder, config.at("logFolder"), L"Ensuring log folder was set.");
                    Assert::AreEqual(sharedContentFolder, config.at("sharedContentFolder"), L"Ensuring shared content folder was set.");
                    Assert::AreEqual(logFolder, GSDK::getLogsDirectory(), L"Ensuring  get log folder works.");
                    Assert::AreEqual(sharedContentFolder, GSDK::getSharedContentDirectory(), L"Ensuring  get shared content folder works.");
                    Assert::AreEqual(certFolder, config.at("certificateFolder"), L"Ensuring cert folder was set.");
                    Assert::AreEqual(std::string("thumbprint1"), config.at("cert1"), L"Ensuring cert1 thumbprint was set.");
                    Assert::AreEqual(std::string("thumbprint2"), config.at("cert2"), L"Ensuring cert2 thumbprint was set.");
                    Assert::AreEqual(titleId, config.at("titleId"), L"Ensuring titleId was set.");
                    Assert::AreEqual(buildId, config.at("buildId"), L"Ensuring buildId was set.");
                    Assert::AreEqual(region, config.at("region"), L"Ensuring region was set.");
                    Assert::AreEqual(std::string("value1"), config.at("key1"), L"Ensuring key1 was set.");
                    Assert::AreEqual(std::string("value2"), config.at("key2"), L"Ensuring key2 was set.");
                    Assert::AreEqual(std::string("1111"), config.at("port1"), L"Ensuring port1 was set.");
                    Assert::AreEqual(std::string("2222"), config.at("port2"), L"Ensuring port2 was set.");
                }

                TEST_METHOD(LogFolderNotSetInitializesFine)
                {
                    GSDKInternal::testConfiguration = std::make_unique<TestConfig>("heartbeatEndpoint", "serverId", "", "sharedContentFolder");
                    GSDK::start();
                    Assert::IsTrue("" == GSDK::getLogsDirectory(), L"Ensuring  get log folder works.");
                }

                TEST_METHOD(SharedContentFolderNotSetInitializesFine)
                {
                    GSDKInternal::testConfiguration = std::make_unique<TestConfig>("heartbeatEndpoint", "serverId", "logFolder", "");
                    GSDK::start();
                    Assert::IsTrue("" == GSDK::getSharedContentDirectory(), L"Ensuring  get shared Content folder works.");
                }

                TEST_METHOD(HeartbeatEndpointNotSetFailsToInitialize)
                {
                    try
                    {
                        GSDKInternal::testConfiguration = std::make_unique<TestConfig>("", "serverId", "logFolder", "sharedContentFolder");
                        GSDK::start();
                        Assert::Fail(L"Did not throw an exception even though the heartbeat endpoint was not set.");
                    }
                    catch (const GSDKInitializationException &ex)
                    {
                        // We expect to throw this exception, since the heartbeat endpoint isn't set
                    }
                }


                TEST_METHOD(ServerIdNotSetFailsToInitialize)
                {
                    try
                    {
                        GSDKInternal::testConfiguration = std::make_unique<TestConfig>("heartbeatEndpoint", "", "logFolder", "sharedContentFolder");
                        GSDK::start();
                        Assert::Fail(L"Did not throw an exception even though the server id was not set.");
                    }
                    catch (const GSDKInitializationException &ex)
                    {
                        // We expect to throw this exception, since the server id isn't set
                    }
                }

                TEST_METHOD(EncodeGameStateAsValidJson)
                {
                    GSDKInternal::testConfiguration = std::make_unique<TestConfig>("heartbeatEndpoint", "serverId", "logFolder", "sharedContentFolder");
                    GSDK::start();

                    // Test Initial State
                    Json::Value jsonHeartbeatRequest = parseJson(GSDKInternal::m_instance->encodeHeartbeatRequest());
                    Assert::AreEqual("Initializing", jsonHeartbeatRequest["CurrentGameState"].asCString(), L"Verifying initial state.");
                    Assert::AreEqual("Healthy", jsonHeartbeatRequest["CurrentGameHealth"].asCString(), L"Verifying default game health (no callback set).");
                    Assert::IsTrue(jsonHeartbeatRequest["CurrentPlayers"].empty(), L"Verifying no connected players have been set.");

                    // Add health and players, and test switching the state
                    GSDK::registerHealthCallback([]() -> bool { return false; });
                    
                    std::vector<Microsoft::Azure::Gaming::ConnectedPlayer> players;
                    players.push_back(Microsoft::Azure::Gaming::ConnectedPlayer("player1"));
                    players.push_back(Microsoft::Azure::Gaming::ConnectedPlayer("player2"));
                    GSDK::updateConnectedPlayers(players);

                    // Note: not using ReadyForPlayers here because we will block forever, once we switch that behavior if the config isn't set we can test it here
                    GSDKInternal::m_instance->setState(GameState::StandingBy);

                    jsonHeartbeatRequest = parseJson(GSDKInternal::m_instance->encodeHeartbeatRequest());
                    Assert::AreEqual("StandingBy", jsonHeartbeatRequest["CurrentGameState"].asCString(), L"Verifying state change.");
                    Assert::AreEqual("Unhealthy", jsonHeartbeatRequest["CurrentGameHealth"].asCString(), L"Verifying game health with callback.");
                    Assert::AreEqual(2u, jsonHeartbeatRequest["CurrentPlayers"].size(), L"Verifying updated connected players.");
                    Assert::AreEqual("player1", jsonHeartbeatRequest["CurrentPlayers"][0]["PlayerId"].asCString(), L"Verifying player1.");
                    Assert::AreEqual("player2", jsonHeartbeatRequest["CurrentPlayers"][1]["PlayerId"].asCString(), L"Verifying player2.");
                }

                TEST_METHOD(DecodeAgentResponseJsonCorrectly)
                {
                    GSDKInternal::testConfiguration = std::make_unique<TestConfig>("heartbeatEndpoint", "serverId", "logFolder", "sharedContentFolder");
                    GSDK::start();

                    time_t maintenanceTime;
                    GSDK::registerMaintenanceCallback([&maintenanceTime](tm t) -> void { maintenanceTime = _mkgmtime(&t); });

                    std::string responseJson = 
                        R"({
                                "operation":"Active",
                                "sessionConfig":
                                {
                                    "sessionId":"eca7e870-da2e-45f9-bb66-30d89064313a",
                                    "sessionCookie":"OreoCookie"
                                },
                                "nextScheduledMaintenanceUtc":"2018-04-12T16:58:30.1458776Z"
                        }")";
                    GSDKInternal::m_instance->decodeHeartbeatResponse(responseJson);

                    // Test heartbeat response handled correctly
                    const std::unordered_map<std::string, std::string> &config = GSDK::getConfigSettings();
                    Assert::IsTrue("eca7e870-da2e-45f9-bb66-30d89064313a" == config.at("sessionId"), L"Verify session id was captured from the heartbeat.");
                    Assert::IsTrue("OreoCookie" == config.at("sessionCookie"), L"Verify session cookie was captured from the heartbeat.");
                    Assert::IsTrue(1523552310LL == maintenanceTime, L"Verify maintenance callback with correct time was called.");
                    Assert::IsTrue(GSDKInternal::m_instance->m_heartbeatRequest.m_currentGameState == GameState::Active, L"Verify state was changed.");
                }

                TEST_METHOD(DecodeAgentResponse_JsonDoesntCrash)
                {
                    GSDKInternal::testConfiguration = std::make_unique<TestConfig>("heartbeatEndpoint", "serverId", "logFolder", "sharedContentFolder");
                    GSDK::start();

                    time_t maintenanceTime;
                    GSDK::registerMaintenanceCallback([&maintenanceTime](tm t) -> void { maintenanceTime = _mkgmtime(&t); });

                    std::string responseJson =
                        R"({
                                "operation": {
                                    "status": "Active"
                                },
                                "sessionConfig":
                                {
                                    "sessionId":"eca7e870-da2e-45f9-bb66-30d89064313a",
                                    "sessionCookie":"OreoCookie"
                                },
                                "nextScheduledMaintenanceUtc":"2018-04-12T16:58:30.1458776Z"
                        }")";
                    GSDKInternal::m_instance->decodeHeartbeatResponse(responseJson);
                }

                TEST_METHOD(DecodeAgentResponse_JsonDoesntCrashWhenInvalidJson)
                {
                    GSDKInternal::testConfiguration = std::make_unique<TestConfig>("heartbeatEndpoint", "serverId", "logFolder", "sharedContentFolder");
                    GSDK::start();

                    time_t maintenanceTime;
                    GSDK::registerMaintenanceCallback([&maintenanceTime](tm t) -> void { maintenanceTime = _mkgmtime(&t); });

                    std::string responseJson =
                        R"({
                                "operation": {
                                    "status": "Active"
                                },
                                "sessionConfig":
                                {
                                    "sessionId":"eca7e870-da2e-45f9-bb66-30d89064313a",
                                    "sessionCookie":"OreoCookie"
                                },
                                "nextScheduledMaintenanceUtc":"2018-04-12T16:58:30.1458776Z",
                        }")";
                    GSDKInternal::m_instance->decodeHeartbeatResponse(responseJson);
                }

                TEST_METHOD(ReturnInitialPlayerListFromJson)
                {
                    GSDKInternal::testConfiguration = std::make_unique<TestConfig>("heartbeatEndpoint", "serverId", "logFolder", "sharedContentFolder");
                    GSDK::start();

                    Assert::AreEqual((size_t)0, GSDK::getInitialPlayers().size(), L"InitialPlayer list is empty before allocation.");

                    std::string responseJson =
                        R"({
                                "operation":"Active",
                                "sessionConfig":
                                {
                                    "sessionId":"eca7e870-da2e-45f9-bb66-30d89064313a",
                                    "sessionCookie":"OreoCookie",
                                    "initialPlayers":
                                    [
                                        "player0",
                                        "player1",
                                        "player2"
                                    ],
                                    "randomList":
                                    [
                                        "item1",
                                        "item2"
                                    ]
                                }
                        }")";
                    GSDKInternal::m_instance->decodeHeartbeatResponse(responseJson);

                    // Test heartbeat response handled correctly
                    const std::vector<std::string> &players = GSDK::getInitialPlayers();
                    Assert::AreEqual((size_t)3, players.size(), L"Player list should now have values.");
                    Assert::AreEqual(std::string("player0"), players[0], L"Verify player0 exists.");
                    Assert::AreEqual(std::string("player1"), players[1], L"Verify player1 exists.");
                    Assert::AreEqual(std::string("player2"), players[2], L"Verify player2 exists.");
                }

                TEST_METHOD(AgentOperationStateChangesHandledCorrectly)
                {
                    GSDKInternal::testConfiguration = std::make_unique<TestConfig>("heartbeatEndpoint", "serverId", "logFolder", "sharedContentFolder");
                    GSDK::start();

                    std::atomic<bool> shutdownCalled = false;
                    GSDK::registerShutdownCallback([&shutdownCalled]() -> void { shutdownCalled = true; });

                    // Call ReadyForPlayers in another thread so we don't block this one
                    std::future<bool> readyForPlayersThread = std::async(std::launch::async, []() -> bool { return GSDK::readyForPlayers(); });

                    // Test state change to Active
                    std::string responseJson =
                        R"({
                                "operation":"Active"
                        }")";
                    GSDKInternal::m_instance->decodeHeartbeatResponse(responseJson);

                    Assert::IsTrue(readyForPlayersThread.get(), L"Verify we were activated and readyForPlayers returned.");

                    // Test state change to some operation we don't know about
                    responseJson =
                        R"({
                                "operation":"TakeOverTheWorld"
                        }")";
                    GSDKInternal::m_instance->decodeHeartbeatResponse(responseJson);

                    Assert::IsTrue(GSDKInternal::m_instance->m_heartbeatRequest.m_currentGameState == GameState::Active, L"Verify state stayed as Active.");

                    // Test state change to some operation we know about but don't support
                    responseJson =
                        R"({
                                "operation":"GetManifest"
                        }")";
                    GSDKInternal::m_instance->decodeHeartbeatResponse(responseJson);

                    Assert::IsTrue(GSDKInternal::m_instance->m_heartbeatRequest.m_currentGameState == GameState::Active, L"Verify state stayed as Active.");

                    // Test state change to Terminate
                    responseJson =
                        R"({
                                "operation":"Terminate"
                        }")";
                    GSDKInternal::m_instance->decodeHeartbeatResponse(responseJson);

                    // Give it a few moments for the callback to finish, since it is run asynchronously
                    int retryCount = 0;
                    while (!shutdownCalled && retryCount < 3)
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        retryCount++;
                    }

                    Assert::IsTrue(shutdownCalled, L"Verify our shutdown callback was called.");
                }

            private:
                Json::Value parseJson(std::string jsonStr)
                {
                    Json::CharReaderBuilder jsonReaderFactory;
                    std::unique_ptr<Json::CharReader> jsonReader(jsonReaderFactory.newCharReader());
                    Json::Value json;
                    JSONCPP_STRING jsonParseErrors;
                    bool parsedSuccessfully = jsonReader->parse(jsonStr.c_str(), jsonStr.c_str() + jsonStr.length(), &json, &jsonParseErrors);

                    Assert::IsTrue(parsedSuccessfully, L"Encoded json state should decode as valid json.");
                    return json;
                }
            };
        }
    }
}