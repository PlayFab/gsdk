namespace PlayFab.MultiplayerAgent.Tests
{
    using System;
    using System.Collections.Generic;
    using System.Globalization;
    using Helpers;
    using Model;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    [TestClass]
    public class SerializationTests
    {
        private static readonly SimpleJsonInstance _jsonInstance = new SimpleJsonInstance();

        [TestMethod]
        public void HeartbeatRequest_Serialization_RoundTrip()
        {
            var request = new HeartbeatRequest
            {
                CurrentGameState = GameState.Active,
                CurrentGameHealth = "Healthy",
                CurrentPlayers = new List<ConnectedPlayer>
                {
                    new ConnectedPlayer("player1"),
                    new ConnectedPlayer("player2")
                }
            };

            string json = _jsonInstance.SerializeObject(request);
            var deserialized = _jsonInstance.DeserializeObject<HeartbeatRequest>(json);

            Assert.AreEqual(GameState.Active, deserialized.CurrentGameState);
            Assert.AreEqual("Healthy", deserialized.CurrentGameHealth);
            Assert.AreEqual(2, deserialized.CurrentPlayers.Count);
            Assert.AreEqual("player1", deserialized.CurrentPlayers[0].PlayerId);
            Assert.AreEqual("player2", deserialized.CurrentPlayers[1].PlayerId);
        }

        [TestMethod]
        public void HeartbeatRequest_InitialState_SerializesCorrectly()
        {
            var request = new HeartbeatRequest
            {
                CurrentGameState = GameState.Initializing,
                CurrentGameHealth = "Healthy",
                CurrentPlayers = new List<ConnectedPlayer>()
            };

            string json = _jsonInstance.SerializeObject(request);
            var deserialized = _jsonInstance.DeserializeObject<HeartbeatRequest>(json);

            Assert.AreEqual(GameState.Initializing, deserialized.CurrentGameState);
            Assert.AreEqual("Healthy", deserialized.CurrentGameHealth);
            Assert.AreEqual(0, deserialized.CurrentPlayers.Count);
        }

        [TestMethod]
        public void HeartbeatResponse_Deserialization_ActiveOperation()
        {
            string responseJson = @"
            {
                ""operation"": ""Active"",
                ""sessionConfig"": {
                    ""sessionId"": ""eca7e870-da2e-45f9-bb66-30d89064313a"",
                    ""sessionCookie"": ""OreoCookie""
                },
                ""nextScheduledMaintenanceUtc"": ""2018-04-12T16:58:30.145Z"",
                ""nextHeartbeatIntervalMs"": 30000
            }";

            var response = _jsonInstance.DeserializeObject<HeartbeatResponse>(responseJson);

            Assert.AreEqual(GameOperation.Active, response.Operation);
            Assert.IsNotNull(response.SessionConfig);
            Assert.AreEqual("eca7e870-da2e-45f9-bb66-30d89064313a", response.SessionConfig.SessionId);
            Assert.AreEqual("OreoCookie", response.SessionConfig.SessionCookie);
            Assert.AreEqual("2018-04-12T16:58:30.145Z", response.NextScheduledMaintenanceUtc);
        }

        [TestMethod]
        public void HeartbeatResponse_Deserialization_TerminateOperation()
        {
            string responseJson = @"
            {
                ""operation"": ""Terminate"",
                ""sessionConfig"": {
                    ""sessionId"": ""test-session""
                }
            }";

            var response = _jsonInstance.DeserializeObject<HeartbeatResponse>(responseJson);
            Assert.AreEqual(GameOperation.Terminate, response.Operation);
        }

        [TestMethod]
        public void HeartbeatResponse_Deserialization_ContinueOperation()
        {
            string responseJson = @"
            {
                ""operation"": ""Continue""
            }";

            var response = _jsonInstance.DeserializeObject<HeartbeatResponse>(responseJson);
            Assert.AreEqual(GameOperation.Continue, response.Operation);
        }

        [TestMethod]
        public void HeartbeatResponse_WithInitialPlayers_Parsed()
        {
            string responseJson = @"
            {
                ""operation"": ""Active"",
                ""sessionConfig"": {
                    ""sessionId"": ""eca7e870-da2e-45f9-bb66-30d89064313a"",
                    ""sessionCookie"": ""OreoCookie"",
                    ""initialPlayers"": [""player0"", ""player1"", ""player2""]
                }
            }";

            var response = _jsonInstance.DeserializeObject<HeartbeatResponse>(responseJson);

            Assert.IsNotNull(response.SessionConfig.InitialPlayers);
            Assert.AreEqual(3, response.SessionConfig.InitialPlayers.Count);
            Assert.AreEqual("player0", response.SessionConfig.InitialPlayers[0]);
            Assert.AreEqual("player1", response.SessionConfig.InitialPlayers[1]);
            Assert.AreEqual("player2", response.SessionConfig.InitialPlayers[2]);
        }

        [TestMethod]
        public void HeartbeatResponse_WithSessionMetadata_Parsed()
        {
            string responseJson = @"
            {
                ""operation"": ""Active"",
                ""sessionConfig"": {
                    ""sessionId"": ""eca7e870-da2e-45f9-bb66-30d89064313a"",
                    ""sessionCookie"": ""OreoCookie"",
                    ""metadata"": {
                        ""testKey"": ""testValue""
                    }
                }
            }";

            var response = _jsonInstance.DeserializeObject<HeartbeatResponse>(responseJson);

            Assert.IsNotNull(response.SessionConfig.Metadata);
            Assert.AreEqual("testValue", response.SessionConfig.Metadata["testKey"]);
        }

        [TestMethod]
        public void HeartbeatResponse_WithMaintenanceSchedule_Parsed()
        {
            string responseJson = @"
            {
                ""operation"": ""Active"",
                ""sessionConfig"": {
                    ""sessionId"": ""eca7e870-da2e-45f9-bb66-30d89064313a""
                },
                ""maintenanceSchedule"": {
                    ""documentIncarnation"": ""IncarnationID"",
                    ""Events"": [
                        {
                            ""eventId"": ""eventID"",
                            ""eventType"": ""Reboot"",
                            ""resourceType"": ""VirtualMachine"",
                            ""Resources"": [""resourceName""],
                            ""eventStatus"": ""Scheduled"",
                            ""notBefore"": ""2018-04-12T16:58:30.145Z"",
                            ""description"": ""eventDescription"",
                            ""eventSource"": ""Platform"",
                            ""durationInSeconds"": 3600
                        }
                    ]
                }
            }";

            var response = _jsonInstance.DeserializeObject<HeartbeatResponse>(responseJson);

            Assert.IsNotNull(response.MaintenanceSchedule);
            Assert.AreEqual("IncarnationID", response.MaintenanceSchedule.DocumentIncarnation);
            Assert.AreEqual(1, response.MaintenanceSchedule.Events.Count);

            var evt = response.MaintenanceSchedule.Events[0];
            Assert.AreEqual("eventID", evt.EventId);
            Assert.AreEqual("Reboot", evt.EventType);
            Assert.AreEqual("VirtualMachine", evt.ResourceType);
            Assert.AreEqual(1, evt.Resources.Count);
            Assert.AreEqual("resourceName", evt.Resources[0]);
            Assert.AreEqual("Scheduled", evt.EventStatus);
            Assert.IsNotNull(evt.NotBefore);
            Assert.AreEqual("eventDescription", evt.Description);
            Assert.AreEqual("Platform", evt.EventSource);
            Assert.AreEqual(3600, evt.DurationInSeconds);
        }

        [TestMethod]
        public void ConnectedPlayer_Serialization_RoundTrip()
        {
            var player = new ConnectedPlayer("testPlayer");
            string json = _jsonInstance.SerializeObject(player);
            var deserialized = _jsonInstance.DeserializeObject<ConnectedPlayer>(json);

            Assert.AreEqual("testPlayer", deserialized.PlayerId);
        }

        [TestMethod]
        public void ConnectedPlayer_DefaultConstructor_NullPlayerId()
        {
            var player = new ConnectedPlayer();
            Assert.IsNull(player.PlayerId);
        }

        [TestMethod]
        public void InvalidJson_DoesNotCrash()
        {
            string responseJson = @"
            {
                ""operation"": ""Active"",
                ""sessionConfig"": {
                    ""sessionId"": ""eca7e870-da2e-45f9-bb66-30d89064313a"",
                    ""sessionCookie"": ""OreoCookie""
                },
                ""nextScheduledMaintenanceUtc"": ""2018-04-12T16:58:30.145Z"",
            }";

            // Should not throw - SimpleJson is lenient with trailing commas
            _jsonInstance.DeserializeObject<HeartbeatResponse>(responseJson);
        }

        [TestMethod]
        public void MaintenanceSchedule_Serialization_RoundTrip()
        {
            var schedule = new MaintenanceSchedule
            {
                DocumentIncarnation = "TestIncarnation",
                Events = new List<MaintenanceEvent>
                {
                    new MaintenanceEvent
                    {
                        EventId = "evt1",
                        EventType = "Reboot",
                        ResourceType = "VirtualMachine",
                        Resources = new List<string> { "resource1" },
                        EventStatus = "Scheduled",
                        Description = "Test event",
                        EventSource = "Platform",
                        DurationInSeconds = 1800
                    }
                }
            };

            string json = _jsonInstance.SerializeObject(schedule);
            var deserialized = _jsonInstance.DeserializeObject<MaintenanceSchedule>(json);

            Assert.AreEqual("TestIncarnation", deserialized.DocumentIncarnation);
            Assert.AreEqual(1, deserialized.Events.Count);
            Assert.AreEqual("evt1", deserialized.Events[0].EventId);
            Assert.AreEqual("Reboot", deserialized.Events[0].EventType);
            Assert.AreEqual(1800, deserialized.Events[0].DurationInSeconds);
        }

        [TestMethod]
        public void GameServerConnectionInfo_Deserialization_Parsed()
        {
            string json = @"
            {
                ""publicIpV4Address"": ""10.0.0.1"",
                ""gamePortsConfiguration"": [
                    {
                        ""name"": ""game"",
                        ""serverListeningPort"": 7777,
                        ""clientConnectionPort"": 30000
                    },
                    {
                        ""name"": ""debug"",
                        ""serverListeningPort"": 8888,
                        ""clientConnectionPort"": 30001
                    }
                ]
            }";

            var connInfo = _jsonInstance.DeserializeObject<GameServerConnectionInfo>(json);
            Assert.AreEqual("10.0.0.1", connInfo.PublicIPv4Address);
        }

        [TestMethod]
        public void GameState_AllValues_SerializeCorrectly()
        {
            foreach (GameState state in Enum.GetValues(typeof(GameState)))
            {
                var request = new HeartbeatRequest { CurrentGameState = state };
                string json = _jsonInstance.SerializeObject(request);
                var deserialized = _jsonInstance.DeserializeObject<HeartbeatRequest>(json);
                Assert.AreEqual(state, deserialized.CurrentGameState,
                    $"GameState {state} should survive round-trip serialization.");
            }
        }

        [TestMethod]
        public void GameOperation_AllValues_DeserializeCorrectly()
        {
            foreach (GameOperation op in Enum.GetValues(typeof(GameOperation)))
            {
                string json = $@"{{ ""operation"": ""{op}"" }}";
                var response = _jsonInstance.DeserializeObject<HeartbeatResponse>(json);
                Assert.AreEqual(op, response.Operation,
                    $"GameOperation {op} should deserialize correctly.");
            }
        }
    }
}
