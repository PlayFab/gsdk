using System;
using System.IO;
using System.Collections;
using System.Collections.Generic;
using NUnit.Framework;
using UnityEngine;
using UnityEngine.TestTools;

using PlayFab;
using PlayFab.MultiplayerAgent.Model;
using PlayFab.MultiplayerAgent.Helpers;
using static UnityEditor.Experimental.GraphView.GraphView;
using System.Threading;
using System.Threading.Tasks;

#if UNIT_TESTING
public class GSDKTests
{
    private readonly string ConfigFilePath = Application.dataPath + "/Tests/testConfig.json";

    private static SimpleJsonInstance _jsonInstance = new SimpleJsonInstance();

    private GSDKConfiguration _testConfig;

    [SetUp]
    public void Setup()
    {
        Environment.SetEnvironmentVariable("PF_TITLE_ID", "testTitleId");
        Environment.SetEnvironmentVariable("PF_BUILD_ID", "testBuildId");
        Environment.SetEnvironmentVariable("PF_REGION", "testRegion");
        Environment.SetEnvironmentVariable("GSDK_CONFIG_FILE", ConfigFilePath);
    }

    [TearDown]
    public void Cleanup()
    {
        PlayFabMultiplayerAgentAPI.ResetAgent();

        _testConfig = null;
        if (File.Exists(ConfigFilePath))
        {
            File.Delete(ConfigFilePath);
        }
    }

    // A Test behaves as an ordinary method
    [Test]
    public void ConfigAllSetInitializesFine()
    {
        _testConfig = new()
        {
            HeartbeatEndpoint = "testEndpoint",
            SessionHostId = "testServerId",
            LogFolder = "testLogFolder",
            SharedContentFolder = "testSharedContentFolder",
            CertificateFolder = "testCertFolder"
        };
        _testConfig.GameCertificates["cert1"] = "thumbprint1";
        _testConfig.GameCertificates["cert2"] = "thumbprint2";
        _testConfig.BuildMetadata["key1"] = "value1";
        _testConfig.BuildMetadata["key2"] = "value2";
        _testConfig.GamePorts["port1"] = "1111";
        _testConfig.GamePorts["port2"] = "2222";

        string json = _jsonInstance.SerializeObject(_testConfig);
        File.WriteAllText(ConfigFilePath, json);

        PlayFabMultiplayerAgentAPI.Start();
        var config = PlayFabMultiplayerAgentAPI.GetConfigSettings();

        Assert.AreEqual("testEndpoint", config["heartbeatEndpoint"], "Ensuring heartbeat endpoint was set.");
        Assert.AreEqual("testServerId", config["serverId"], "Ensuring server id was set.");
        Assert.AreEqual("testLogFolder", config["logFolder"], "Ensuring log folder was set.");
        Assert.AreEqual("testSharedContentFolder", config["sharedContentFolder"], "Ensuring shared content folder was set.");
        Assert.AreEqual("testCertFolder", config["certificateFolder"], "Ensuring cert folder was set.");
        Assert.AreEqual("thumbprint1", config["cert1"], "Ensuring cert1 thumbprint was set.");
        Assert.AreEqual("thumbprint2", config["cert2"], "Ensuring cert2 thumbprint was set.");
        Assert.AreEqual("testTitleId", config["titleId"], "Ensuring titleId was set.");
        Assert.AreEqual("testBuildId", config["buildId"], "Ensuring buildId was set.");
        Assert.AreEqual("testRegion", config["region"], "Ensuring region was set.");
        Assert.AreEqual("value1", config["key1"], "Ensuring key1 was set.");
        Assert.AreEqual("value2", config["key2"], "Ensuring key2 was set.");
        Assert.AreEqual("1111", config["port1"], "Ensuring port1 was set.");
        Assert.AreEqual("2222", config["port2"], "Ensuring port2 was set.");
    }

    [Test]
    public void LogFolderNotSetInitializesFine()
    {
        _testConfig = new()
        {
            HeartbeatEndpoint = "testEndpoint",
            SessionHostId = "testServerId",
            SharedContentFolder = "testSharedContentFolder"
        };

        string json = _jsonInstance.SerializeObject(_testConfig);
        File.WriteAllText(ConfigFilePath, json);

        PlayFabMultiplayerAgentAPI.Start();
    }

    [Test]
    public void SharedContentFolderNotSetInitializesFine()
    {
        _testConfig = new()
        {
            HeartbeatEndpoint = "testEndpoint",
            SessionHostId = "testServerId",
            LogFolder = "testLogFolder"
        };

        string json = _jsonInstance.SerializeObject(_testConfig);
        File.WriteAllText(ConfigFilePath, json);

        PlayFabMultiplayerAgentAPI.Start();
    }

    [Test]
    public void HeartbeatEndpointNotSetFailsToInitialize()
    {
        _testConfig = new()
        {
            SessionHostId = "testServerId",
            LogFolder = "testLogFolder",
            SharedContentFolder = "testSharedContentFolder"
        };

        string json = _jsonInstance.SerializeObject(_testConfig);
        File.WriteAllText(ConfigFilePath, json);

        PlayFabMultiplayerAgentAPI.Start();
        PlayFabMultiplayerAgentAPI.SendHeartBeatRequest();
    }

    [Test]
    public void EncodeGameStateAsValidJson()
    {
        _testConfig = new()
        {
            HeartbeatEndpoint = "testEndpoint",
            SessionHostId = "testServerId",
            LogFolder = "testLogFolder",
            SharedContentFolder = "testSharedContentFolder"
        };

        string json = _jsonInstance.SerializeObject(_testConfig);
        File.WriteAllText(ConfigFilePath, json);

        PlayFabMultiplayerAgentAPI.Start();

        var testHeartbeat1 = _jsonInstance.DeserializeObject<HeartbeatRequest>(_jsonInstance.SerializeObject(PlayFabMultiplayerAgentAPI.CurrentState));
        Assert.AreEqual(GameState.Initializing, testHeartbeat1.CurrentGameState, "Verifying initial state.");
        Assert.AreEqual("Healthy", testHeartbeat1.CurrentGameHealth, "Verifying default game health (no callback set).");
        Assert.AreEqual(0, testHeartbeat1.CurrentPlayers.Count, "Verifying no connected players have been set.");

        List<ConnectedPlayer> players = new()
        {
            new("player1"),
            new("player2")
        };

        PlayFabMultiplayerAgentAPI.UpdateConnectedPlayers(players);
        PlayFabMultiplayerAgentAPI.CurrentState.CurrentGameHealth = "Unhealthy";
        PlayFabMultiplayerAgentAPI.CurrentState.CurrentGameState = GameState.StandingBy;
        
        var testHeartbeat2 = _jsonInstance.DeserializeObject<HeartbeatRequest>(_jsonInstance.SerializeObject(PlayFabMultiplayerAgentAPI.CurrentState));
        Assert.AreEqual(GameState.StandingBy, testHeartbeat2.CurrentGameState, "Verifying state change.");
        Assert.AreEqual("Unhealthy", testHeartbeat2.CurrentGameHealth, "Verifying game health with callback.");
        Assert.AreEqual(2, testHeartbeat2.CurrentPlayers.Count, "Verifying updated connected players.");
        Assert.AreEqual("player1", testHeartbeat2.CurrentPlayers[0].PlayerId, "Verifying player1.");
        Assert.AreEqual("player2", testHeartbeat2.CurrentPlayers[1].PlayerId, "Verifying player2.");
    }

    [Test]
    public void DecodeAgentResponseJsonCorrectly()
    {
        _testConfig = new()
        {
            HeartbeatEndpoint = "testEndpoint",
            SessionHostId = "testServerId",
            LogFolder = "testLogFolder",
            SharedContentFolder = "testSharedContentFolder"
        };

        string json = _jsonInstance.SerializeObject(_testConfig);
        File.WriteAllText(ConfigFilePath, json);

        PlayFabMultiplayerAgentAPI.Start();

        DateTime maintenanceTime = default;
        PlayFabMultiplayerAgentAPI.OnMaintenanceCallback += (DateTime? nextTime) =>
        {
            maintenanceTime = nextTime.Value;
        };

        string responseJson = @"
            {
                ""operation"" : ""Active"",
                ""sessionConfig"" : 
                {
                    ""sessionId"" : ""eca7e870-da2e-45f9-bb66-30d89064313a"",
                    ""sessionCookie"" : ""OreoCookie""
                },
                ""nextScheduledMaintenanceUtc"" : ""2018-04-12T16:58:30.1458776Z"",
                ""nextHeartbeatIntervalMs"" : 30000
            }";

        var response = _jsonInstance.DeserializeObject<HeartbeatResponse>(responseJson);
        PlayFabMultiplayerAgentAPI.ProcessAgentResponse(response);
        var config = PlayFabMultiplayerAgentAPI.GetConfigSettings();
        var expectedTime = new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc);
        expectedTime = expectedTime.AddSeconds(1523552310).AddTicks(1458776);

        Assert.AreEqual("eca7e870-da2e-45f9-bb66-30d89064313a", config["sessionId"], "Verify session id was captured from the heartbeat.");
        Assert.AreEqual("OreoCookie", config["sessionCookie"], "Verify session cookie was captured from the heartbeat.");
        Assert.AreEqual(expectedTime, maintenanceTime, "Verify maintenance callback with correct time was called.");
        Assert.AreEqual(GameState.Active, PlayFabMultiplayerAgentAPI.CurrentState.CurrentGameState, "Verify state was changed.");
    }

    [Test]
    public void GameState_MaintV2_CallbackInvoked()
    {
        _testConfig = new()
        {
            HeartbeatEndpoint = "testEndpoint",
            SessionHostId = "testServerId",
            LogFolder = "testLogFolder",
            SharedContentFolder = "testSharedContentFolder"
        };

        string json = _jsonInstance.SerializeObject(_testConfig);
        File.WriteAllText(ConfigFilePath, json);

        PlayFabMultiplayerAgentAPI.Start();

        MaintenanceSchedule schedule = new();
        PlayFabMultiplayerAgentAPI.OnMaintenanceV2Callback += (MaintenanceSchedule maintenanceSchedule) =>
        {
            schedule = maintenanceSchedule;
        };

        string responseJson = @"
            {
                ""operation"" : ""Active"",
                ""sessionConfig"" : 
                {
                    ""sessionId"" : ""eca7e870-da2e-45f9-bb66-30d89064313a"",
                    ""sessionCookie"" : ""OreoCookie""
                },
                ""maintenanceSchedule"": 
                {
                    ""DocumentIncarnation"": ""IncarnationID"",
                    ""Events"": 
                    [
                        {
                            ""EventId"": ""eventID"",
                            ""EventType"": ""Reboot"",
                            ""ResourceType"": ""VirtualMachine"",
                            ""Resources"": 
                            [
                                ""resourceName""
                            ],
                            ""EventStatus"": ""Scheduled"",
                            ""NotBefore"": ""2018-04-12T16:58:30.1458776Z"",
                            ""Description"": ""eventDescription"",
                            ""EventSource"": ""Platform"",
                            ""DurationInSeconds"": 3600
                        }
                    ]
                },
                ""nextHeartbeatIntervalMs"" : 30000
            }";

        var response = _jsonInstance.DeserializeObject<HeartbeatResponse>(responseJson);
        PlayFabMultiplayerAgentAPI.ProcessAgentResponse(response);

        var expectedTime = new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc);
        expectedTime = expectedTime.AddSeconds(1523552310).AddTicks(1458776);

        Assert.AreEqual("IncarnationID", schedule.DocumentIncarnation, "Verify maintenance V2 callback with correct document incarnation was called.");
        Assert.AreEqual(1, schedule.Events.Count, "Verify maintenance V2 callback with correct event list size was called.");
        Assert.AreEqual("eventID", schedule.Events[0].EventId, "Verify maintenance V2 callback with correct event id was called.");
        Assert.AreEqual("Reboot", schedule.Events[0].EventType, "Verify maintenance V2 callback with correct event type was called.");
        Assert.AreEqual("VirtualMachine", schedule.Events[0].ResourceType, "Verify maintenance V2 callback with correct resource type was called.");
        Assert.AreEqual(1, schedule.Events[0].Resources.Count, "Verify maintenance V2 callback with correct resource list size was called.");
        Assert.AreEqual("resourceName", schedule.Events[0].Resources[0], "Verify maintenance V2 callback with correct resource was called.");
        Assert.AreEqual("Scheduled", schedule.Events[0].EventStatus, "Verify maintenance V2 callback with correct status was called.");
        Assert.AreEqual(expectedTime, schedule.Events[0].NotBefore, "Verify maintenance V2 callback with correct time was called.");
        Assert.AreEqual("eventDescription", schedule.Events[0].Description, "Verify maintenance V2 callback with correct description was called.");
        Assert.AreEqual("Platform", schedule.Events[0].EventSource, "Verify maintenance V2 callback with correct source was called.");
        Assert.AreEqual(3600, schedule.Events[0].DurationInSeconds, "Verify maintenance V2 callback with correct duration was called.");
    }

    [Test]
    public void JsonDoesntCrashWhenInvalidJson()
    {
        string responseJson = @"
            {
                ""operation"" : ""Active"",
                ""sessionConfig"" : 
                {
                    ""sessionId"" : ""eca7e870-da2e-45f9-bb66-30d89064313a"",
                    ""sessionCookie"" : ""OreoCookie""
                },
                ""nextScheduledMaintenanceUtc"" : ""2018-04-12T16:58:30.1458776Z"",
            }";

        _jsonInstance.DeserializeObject<HeartbeatResponse>(responseJson);

        Assert.Pass("No crash.");
    }

    [Test]
    public void ReturnInitialPlayerListFromJson()
    {
        _testConfig = new()
        {
            HeartbeatEndpoint = "testEndpoint",
            SessionHostId = "testServerId",
            LogFolder = "testLogFolder",
            SharedContentFolder = "testSharedContentFolder"
        };

        string json = _jsonInstance.SerializeObject(_testConfig);
        File.WriteAllText(ConfigFilePath, json);

        PlayFabMultiplayerAgentAPI.Start();

        Assert.AreEqual(0, PlayFabMultiplayerAgentAPI.GetInitialPlayers().Count, "InitialPlayer list is empty before allocation.");

        string responseJson = @"
            {
                ""operation"" : ""Active"",
                ""sessionConfig"" : 
                {
                    ""sessionId"" : ""eca7e870-da2e-45f9-bb66-30d89064313a"",
                    ""sessionCookie"" : ""OreoCookie"",
                    ""initialPlayers"" : 
                    [
                        ""player0"",
                        ""player1"",
                        ""player2""
                    ],
                    ""randomList"" : 
                    [
                        ""item1"",
                        ""item2""
                    ]
                }
            }";

        var response = _jsonInstance.DeserializeObject<HeartbeatResponse>(responseJson);
        PlayFabMultiplayerAgentAPI.ProcessAgentResponse(response);

        var players = PlayFabMultiplayerAgentAPI.GetInitialPlayers();
        Assert.AreEqual(3, players.Count, "Player list should now have values.");
        Assert.AreEqual("player0", players[0], "Verify player0 exists.");
        Assert.AreEqual("player1", players[1], "Verify player1 exists.");
        Assert.AreEqual("player2", players[2], "Verify player2 exists.");
    }

    [Test]
    public void ReturnSessionMetadataFromJson()
    {
        _testConfig = new()
        {
            HeartbeatEndpoint = "testEndpoint",
            SessionHostId = "testServerId",
            LogFolder = "testLogFolder",
            SharedContentFolder = "testSharedContentFolder"
        };

        string json = _jsonInstance.SerializeObject(_testConfig);
        File.WriteAllText(ConfigFilePath, json);

        PlayFabMultiplayerAgentAPI.Start();

        Assert.AreEqual(0, PlayFabMultiplayerAgentAPI.GetInitialPlayers().Count, "InitialPlayer list is empty before allocation.");

        string responseJson = @"
            {
                ""operation"":""Active"",
                ""sessionConfig"":
                {
                    ""sessionId"":""eca7e870-da2e-45f9-bb66-30d89064313a"",
                    ""sessionCookie"":""OreoCookie"",
                    ""metadata"":
                    {
                        ""testKey"": ""testValue""
                    }
                }
            }";

        var response = _jsonInstance.DeserializeObject<HeartbeatResponse>(responseJson);
        PlayFabMultiplayerAgentAPI.ProcessAgentResponse(response);

        var configSettings = PlayFabMultiplayerAgentAPI.GetConfigSettings();

        Assert.AreEqual("testValue", configSettings["testKey"], "Verify metadata exists.");
    }

    [Test]
    public void AgentOperationStateChangesHandledCorrectly()
    {
        _testConfig = new()
        {
            HeartbeatEndpoint = "testEndpoint",
            SessionHostId = "testServerId",
            LogFolder = "testLogFolder",
            SharedContentFolder = "testSharedContentFolder"
        };

        string json = _jsonInstance.SerializeObject(_testConfig);
        File.WriteAllText(ConfigFilePath, json);

        PlayFabMultiplayerAgentAPI.Start();

        bool shutdownCalled = false;
        PlayFabMultiplayerAgentAPI.OnShutDownCallback += () =>
        {
            shutdownCalled = true;
        };

        PlayFabMultiplayerAgentAPI.ReadyForPlayers();

        // Test state change to Active
        string responseJson = @"
            {
                ""operation"":""Active"",
                ""sessionConfig"" : 
                {
                    ""sessionId"" : ""eca7e870-da2e-45f9-bb66-30d89064313a"",
                    ""sessionCookie"" : ""OreoCookie""
                },
            }";

        var response = _jsonInstance.DeserializeObject<HeartbeatResponse>(responseJson);
        PlayFabMultiplayerAgentAPI.ProcessAgentResponse(response);

        Assert.AreEqual(GameState.Active, PlayFabMultiplayerAgentAPI.CurrentState.CurrentGameState, "Verify state was changed.");

        // Test state change to some operation we know about but don't support
        responseJson = @"
            {
                ""operation"":""Invalid"",
                ""sessionConfig"" : 
                {
                    ""sessionId"" : ""eca7e870-da2e-45f9-bb66-30d89064313a"",
                    ""sessionCookie"" : ""OreoCookie""
                },
            }";

        response = _jsonInstance.DeserializeObject<HeartbeatResponse>(responseJson);
        PlayFabMultiplayerAgentAPI.ProcessAgentResponse(response);

        Assert.AreEqual(GameState.Active, PlayFabMultiplayerAgentAPI.CurrentState.CurrentGameState, "Verify state stayed as Active.");

        // Test state change to Terminate
        responseJson = @"
            {
                ""operation"":""Terminate"",
                ""sessionConfig"" : 
                {
                    ""sessionId"" : ""eca7e870-da2e-45f9-bb66-30d89064313a"",
                    ""sessionCookie"" : ""OreoCookie""
                },
            }";

        response = _jsonInstance.DeserializeObject<HeartbeatResponse>(responseJson);
        PlayFabMultiplayerAgentAPI.ProcessAgentResponse(response);

        Thread.Sleep(1000);

        Assert.IsTrue(shutdownCalled, "Verify shutdown callback was called.");
    }

    // A UnityTest behaves like a coroutine in Play Mode. In Edit Mode you can use
    // `yield return null;` to skip a frame.
    [UnityTest]
    public IEnumerator GSDKTestsWithEnumeratorPasses()
    {
        // Use the Assert class to test conditions.
        // Use yield to skip a frame.
        yield return null;
    }
}
#endif