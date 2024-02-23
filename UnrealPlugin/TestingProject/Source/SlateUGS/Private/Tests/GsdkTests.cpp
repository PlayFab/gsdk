// Copyright (C) Microsoft Corporation. All rights reserved.

#if (WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR)
#include "GsdkTests.h"
#include "PlayFabGSDK.h"
#include "GSDKUtils.h"
#include "TestConfiguration.h"

BEGIN_DEFINE_SPEC(AutomationSpec, "-GSDK", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
const FString ConfigFilePath = FPaths::Combine(FPaths::ProjectDir(), TEXT("/Tests/testConfig.json"));
const FString heartbeatEndpoint = "testEndpoint";
const FString serverId = "testServerId";
const FString logFolder = "testLogFolder";
const FString sharedContentFolder = "testSharedContentFolder";
const FString certFolder = "testCertFolder";
const FString titleId = "testTitleId";
const FString buildId = "testBuildId";
const FString region = "testRegion";
TestConfiguration config;
FEventRef readyForPlayers{ EEventMode::ManualReset };
FEventRef shutdownCalled{ EEventMode::ManualReset };
void SerializeConfigAndStartModule();
END_DEFINE_SPEC(AutomationSpec)

void AutomationSpec::SerializeConfigAndStartModule()
{
    config.SerializeToFile(ConfigFilePath);
    FPlayFabGSDKModule::Get().ManualStartupModule();
}

void AutomationSpec::Define()
{
    Describe("Tests", [this]()
        {
            BeforeEach([this]()
                {
                    FPlatformMisc::SetEnvironmentVar(TEXT("PF_TITLE_ID"), TEXT("testTitleId"));
                    FPlatformMisc::SetEnvironmentVar(TEXT("PF_BUILD_ID"), TEXT("testBuildId"));
                    FPlatformMisc::SetEnvironmentVar(TEXT("PF_REGION"), TEXT("testRegion"));
                    FPlatformMisc::SetEnvironmentVar(TEXT("GSDK_CONFIG_FILE"), *ConfigFilePath);
                    
                    config = TestConfiguration();
                    config.SetHeartbeatEndpoint(heartbeatEndpoint);
                    config.SetServerId(serverId);
                    config.SetLogFolder(logFolder);
                    config.SetSharedContentFolder(sharedContentFolder);
                });

            AfterEach([this]()
                {
                    FPlayFabGSDKModule::Get().ResetInternalState();

                    if (FPaths::FileExists(ConfigFilePath))
                    {
                        FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*ConfigFilePath);
                    }
                });

            It("ConfigAllSetInitializesFine", [this]()
                {
                    config.SetCertificateFolder(certFolder);

                    TMap<FString, FString> gameCerts = TMap<FString, FString>();
                    gameCerts.Add("cert1", "thumbprint1");
                    gameCerts.Add("cert2", "thumbprint2");
                    config.SetGameCertificates(gameCerts);

                    TMap<FString, FString> metadata = TMap<FString, FString>();
                    metadata.Add("key1", "value1");
                    metadata.Add("key2", "value2");
                    config.SetBuildMetadata(metadata);

                    TMap<FString, FString> ports = TMap<FString, FString>();
                    ports.Add("port1", "1111");
                    ports.Add("port2", "2222");
                    config.SetGamePorts(ports);

                    SerializeConfigAndStartModule();

                    TestEqual("heartbeat endpoint", heartbeatEndpoint, FPlayFabGSDKModule::Get().GetConfigValue("gsmsBaseUrl"));
                    TestEqual("cert folder", certFolder, FPlayFabGSDKModule::Get().GetConfigValue("certificateFolder"));
                    TestEqual("cert1 thumbprint", "thumbprint1", FPlayFabGSDKModule::Get().GetConfigValue("cert1"));
                    TestEqual("cert2 thumbprint", "thumbprint2", FPlayFabGSDKModule::Get().GetConfigValue("cert2"));
                    TestEqual("port1", "1111", FPlayFabGSDKModule::Get().GetConfigValue("port1"));
                    TestEqual("port2", "2222", FPlayFabGSDKModule::Get().GetConfigValue("port2"));

                    TestEqual("server id", serverId, UGSDKUtils::GetServerId());
                    TestEqual("titleId", titleId, UGSDKUtils::GetTitleId());
                    TestEqual("buildId", buildId, UGSDKUtils::GetBuildId());
                    TestEqual("region", region, UGSDKUtils::GetRegionName());
                    TestEqual("key1", "value1", UGSDKUtils::GetMetaDataValue("key1"));
                    TestEqual("key2", "value2", UGSDKUtils::GetMetaDataValue("key2"));
                    TestEqual("log folder", logFolder, UGSDKUtils::GetLogsDirectory());
                    TestEqual("shared content folder", sharedContentFolder, UGSDKUtils::GetSharedContentDirectory());
                });

            It("LogFolderNotSetInitializesFine", [this]()
                {
                    FString emptyString;
                    emptyString.Empty();

                    config.SetLogFolder(emptyString);

                    SerializeConfigAndStartModule();

                    TestEqual("get log folder works", "", UGSDKUtils::GetLogsDirectory());
                });

            It("SharedContentFolderNotSetInitializesFine", [this]()
                {
                    FString emptyString;
                    emptyString.Empty();

                    config.SetSharedContentFolder(emptyString);
                    
                    SerializeConfigAndStartModule();

                    TestEqual("get shared content folder works", "", UGSDKUtils::GetSharedContentDirectory());
                });

            It("EncodeGameStateAsValidJson", [this]()
                {
                    SerializeConfigAndStartModule();

                    // Test Initial State
                    TSharedPtr<FJsonObject> jsonHeartbeatRequest = MakeShareable(new FJsonObject());
                    TSharedRef<TJsonReader<>> jsonReader = TJsonReaderFactory<>::Create(FPlayFabGSDKModule::Get().GSDKInternal->EncodeHeartbeatRequest());
                    if (!FJsonSerializer::Deserialize(jsonReader, jsonHeartbeatRequest))
                    {
                        TestTrue("Failed to parse heartbeat request", false);
                        return;
                    }

                    TestEqual("Verifying initial state.", jsonHeartbeatRequest->GetStringField("CurrentGameState"), "Initializing");
                    TestEqual("Verifying default game health (no callback set).", jsonHeartbeatRequest->GetStringField("CurrentGameHealth"), "Healthy");
                    TestTrue("Verifying no connected players have been set.", jsonHeartbeatRequest->GetArrayField("CurrentPlayers").IsEmpty());

                    // Add health and players, and test switching the state
                    FPlayFabGSDKModule::Get().OnHealthCheck.BindLambda([]() -> bool { return false; });

                    TArray<FConnectedPlayer> players;
                    players.Add(FConnectedPlayer{ "player1" });
                    players.Add(FConnectedPlayer{ "player2" });
                    UGSDKUtils::UpdateConnectedPlayers(players);

                    // Note: not using ReadyForPlayers here because we will block forever, once we switch that behavior if the config isn't set we can test it here
                    FPlayFabGSDKModule::Get().GSDKInternal->SetState(EGameState::StandingBy);

                    jsonReader = TJsonReaderFactory<>::Create(FPlayFabGSDKModule::Get().GSDKInternal->EncodeHeartbeatRequest());
                    if (!FJsonSerializer::Deserialize(jsonReader, jsonHeartbeatRequest))
                    {
                        TestTrue("Failed to parse heartbeat request", false);
                        return;
                    }

                    TestEqual("Verifying state change.", jsonHeartbeatRequest->GetStringField("CurrentGameState"), "StandingBy");
                    TestEqual("Verifying game health with callback.", jsonHeartbeatRequest->GetStringField("CurrentGameHealth"), "Unhealthy");
                    auto playersArray = jsonHeartbeatRequest->GetArrayField("CurrentPlayers");
                    TestEqual("Verifying updated connected players.", playersArray.Num(), 2u);
                    TestEqual("Verifying player1.", playersArray[0]->AsObject()->GetStringField("PlayerId"), "player1");
                    TestEqual("Verifying player2.", playersArray[1]->AsObject()->GetStringField("PlayerId"), "player2");
                });

            It("DecodeAgentResponseJsonCorrectly", [this]()
                {
                    SerializeConfigAndStartModule();

                    FDateTime maintenanceTime;
                    FPlayFabGSDKModule::Get().OnMaintenance.BindLambda([&maintenanceTime](const FDateTime& time) -> void { maintenanceTime = time; });

                    FString responseJson =
                        R"({
                                "operation":"Active",
                                "sessionConfig":
                                {
                                    "sessionId":"eca7e870-da2e-45f9-bb66-30d89064313a",
                                    "sessionCookie":"OreoCookie"
                                },
                                "nextScheduledMaintenanceUtc":"2018-04-12T16:58:30.1458776Z",
                                "nextHeartbeatIntervalMs":30000
                        })";

                    FPlayFabGSDKModule::Get().GSDKInternal->DecodeHeartbeatResponse(responseJson);

                    // Test heartbeat response handled correctly
                    EGameState gameState = FPlayFabGSDKModule::Get().GSDKInternal->GetHeartbeatRequest().CurrentGameState;
                    TestEqual("Verify session id was captured from the heartbeat.", UGSDKUtils::GetMatchId(), "eca7e870-da2e-45f9-bb66-30d89064313a");
                    TestEqual("Verify session cookie was captured from the heartbeat.", UGSDKUtils::GetMatchSessionCookie(), "OreoCookie");
                    // TODO: Consider later
                    // TestEqual("Verify maintenance callback with correct time was called.", maintenanceTime.GetTicks(), 1523552310LL);
                    TestEqual("Verify state was changed.", EGameState::Active, gameState);
                    TestEqual("Verify heartbeat interval was captured from the heartbeat", 30000, FPlayFabGSDKModule::Get().GSDKInternal->NextHeartbeatIntervalMs);
                });

            It("JsonDoesntCrashWhenInvalidJson", [this]()
                {
                    SerializeConfigAndStartModule();

                    FString responseJson =
                        R"({
                                "operation": {
                                    "status": "Active"
                                },
                                "sessionConfig":
                                {
                                    "sessionId":"eca7e870-da2e-45f9-bb66-30d89064313a",
                                    "sessionCookie":"OreoCookie"
                                }
                        })";

                    AddExpectedError("Json Value of type 'Object' used as a 'String'");
                    AddExpectedError("An error occured while processing heartbeat operation.");
                    AddExpectedError("Active");

                    FPlayFabGSDKModule::Get().GSDKInternal->DecodeHeartbeatResponse(responseJson);
                });

            It("ReturnInitialPlayerListFromJson", [this]()
                {
                    SerializeConfigAndStartModule();

                    TestEqual("InitialPlayer list is empty before allocation.", UGSDKUtils::GetInitialPlayers().Num(), 0);

                    FString responseJson =
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
                        })";

                    FPlayFabGSDKModule::Get().GSDKInternal->DecodeHeartbeatResponse(responseJson);

                    // Test heartbeat response handled correctly
                    auto players = UGSDKUtils::GetInitialPlayers();
                    TestEqual("Player list should now have values.", players.Num(), 3);
                    TestEqual("Verify player0 exists.", players[0], "player0");
                    TestEqual("Verify player1 exists.", players[1], "player1");
                    TestEqual("Verify player2 exists.", players[2], "player2");
                });

            It("ReturnSessionMetadataFromJson", [this]()
                {
                    SerializeConfigAndStartModule();

                    TestEqual("InitialPlayer list is empty before allocation.", UGSDKUtils::GetInitialPlayers().Num(), 0);

                    FString responseJson =
                        R"({
                                "operation":"Active",
                                "sessionConfig":
                                {
                                    "sessionId":"eca7e870-da2e-45f9-bb66-30d89064313a",
                                    "sessionCookie":"OreoCookie",
                                    "metadata":
                                    {
                                        "testKey": "testValue"
                                    }
                                }
                        })";

                    FPlayFabGSDKModule::Get().GSDKInternal->DecodeHeartbeatResponse(responseJson);

                    // Test heartbeat response handled correctly
                    TestEqual("Ensuring session metadata was set.", UGSDKUtils::GetMetaDataValue("testKey"), "testValue");
                });

            It("AgentOperationStateChangesHandledCorrectly", [this]()
                {
                    SerializeConfigAndStartModule();

                    FPlayFabGSDKModule::Get().OnReadyForPlayers.BindLambda([this]() -> void { readyForPlayers->Trigger(); });

                    FPlayFabGSDKModule::Get().OnShutdown.BindLambda([this]() -> void { shutdownCalled->Trigger(); });

                    // Call ReadyForPlayers in another thread so we don't block this one
                    UGSDKUtils::ReadyForPlayers();

                    // Test state change to Active
                    FString responseJson =
                        R"({
                                "operation":"Active"
                        })";

                    FPlayFabGSDKModule::Get().GSDKInternal->DecodeHeartbeatResponse(responseJson);

                    TestTrue("Verify we were activated and readyForPlayers returned.", readyForPlayers->Wait(FTimespan(0, 1, 0)));

                    // Test state change to some operation we don't know about
                    responseJson =
                        R"({
                                "operation":"TakeOverTheWorld"
                        })";

                    AddExpectedError("An error occured while processing heartbeat operation.");
                    AddExpectedError("TakeOverTheWorld");

                    FPlayFabGSDKModule::Get().GSDKInternal->DecodeHeartbeatResponse(responseJson);

                    EGameState gameState = FPlayFabGSDKModule::Get().GSDKInternal->GetHeartbeatRequest().CurrentGameState;
                    TestEqual("Verify state stayed as Active.", gameState, EGameState::Active);

                    // Test state change to some operation we know about but don't support
                    responseJson =
                        R"({
                                "operation":"GetManifest"
                        })";

                    AddExpectedError("Unhandled operation received:");

                    FPlayFabGSDKModule::Get().GSDKInternal->DecodeHeartbeatResponse(responseJson);

                    gameState = FPlayFabGSDKModule::Get().GSDKInternal->GetHeartbeatRequest().CurrentGameState;
                    TestEqual("Verify state stayed as Active.", gameState, EGameState::Active);

                    // Test state change to Terminate
                    responseJson =
                        R"({
                                "operation":"Terminate"
                        })";

                    AddExpectedError("Termination");

                    FPlayFabGSDKModule::Get().GSDKInternal->DecodeHeartbeatResponse(responseJson);

                    TestTrue("Verify our shutdown callback was called.", shutdownCalled->Wait(FTimespan(0, 1, 0)));
                });
        });
}
#endif
