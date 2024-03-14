namespace Microsoft.Playfab.Gaming.GSDK.CSharp.Test
{
    using System;
    using System.Collections.Generic;
    using System.Globalization;
    using System.Threading;
    using System.Threading.Tasks;
    using FluentAssertions;
    using Model;
    using Moq;
    using Newtonsoft.Json;
    using VisualStudio.TestTools.UnitTesting;

    [TestClass]
    public class InternalSdkTests
    {
        private Mock<IHttpClientFactory> _mockHttpClientFactory;
        private Mock<IHttpClient> _mockHttpClient;
        private Mock<ISystemOperations> _mockSystemOperations;

        private const string ConfigFilePath = @"D:\gsdkConfig.json";
        private object _testConfiguration;

        [TestInitialize]
        public void TestInitialize()
        {
            _mockHttpClient = new Mock<IHttpClient>();
            _mockHttpClientFactory = new Mock<IHttpClientFactory>();
            _mockSystemOperations = new Mock<ISystemOperations>();

            _mockSystemOperations.Setup(x => x.GetEnvironmentVariableValue(GameserverSDK.GsdkConfigFileEnvVarKey)).Returns(ConfigFilePath);
            _mockSystemOperations.Setup(op => op.FileExists(ConfigFilePath)).Returns(true);
            _mockSystemOperations.Setup(op => op.FileReadAllText(ConfigFilePath)).Returns(() => JsonConvert.SerializeObject(_testConfiguration));

            _mockHttpClient.Setup(x => x.SendInfoAsync(It.IsAny<string>())).Returns(Task.CompletedTask);
            _mockHttpClient.Setup(x => x.SendHeartbeatAsync(It.IsAny<HeartbeatRequest>()))
                .Returns(Task.FromResult(new HeartbeatResponse()));
            _mockHttpClientFactory.Setup(x => x.CreateInstance(It.IsAny<string>())).Returns(() => _mockHttpClient.Object);
        }

        [TestMethod]
        public void Start_InvalidConfiguration_Throws()
        {
            _testConfiguration = new { };

            Action action = () => new InternalSdk(_mockSystemOperations.Object).Start();
            action.Should().Throw<GSDKInitializationException>().WithMessage($"*{ConfigFilePath}*");
        }

        [TestMethod]
        public void Start_ValidConfiguration_Starts()
        {
            _testConfiguration = new
            {
                heartbeatEndpoint = "heartbeatendpoint",
                sessionHostId = "serverid",
                gameCertificates = new Dictionary<string, string>
                {
                    {"cert1", "thumb1"},
                    {"cert2", "thumb2"},
                    {"cert3", "thumb3"}
                },
                buildMetadata = new Dictionary<string, string>
                {
                    {"property1", "value1"},
                    {"property2", "value2"},
                    {"property3", "value3"}
                },
                gamePorts = new Dictionary<string, string>
                {
                    {"8080", "port1"}
                }
            };

            var sdk = new InternalSdk(_mockSystemOperations.Object, _mockHttpClientFactory.Object);
            sdk.Start(false);
            sdk.State.Should().Be(GameState.Initializing);
            sdk.ConfigMap.Should().NotBeNull();
            sdk.ConfigMap.Should().HaveCountGreaterThan(0);
            sdk.ConnectedPlayers.Should().NotBeNull();
            sdk.ConnectedPlayers.Should().HaveCount(0);
        }

        [TestMethod]
        public void GameStateToActive_ActiveReturned_StateUpdated()
        {
            _testConfiguration = new
            {
                heartbeatEndpoint = "heartbeatendpoint",
                sessionHostId = "serverid"
            };

            _mockHttpClient.Setup(x => x.SendHeartbeatAsync(It.IsAny<HeartbeatRequest>()))
                .ReturnsAsync(new HeartbeatResponse
                {
                    Operation = GameOperation.Active
                });

            var sdk = new InternalSdk(_mockSystemOperations.Object, _mockHttpClientFactory.Object);
            sdk.Start(false);

            sdk.State = GameState.StandingBy;
            sdk.TransitionToActiveEvent.WaitOne();
            sdk.State.Should().Be(GameState.Active);

        }

        [TestMethod]
        public void GameState_TerminateReturned_StateUpdated()
        {
            _testConfiguration = new
            {
                heartbeatEndpoint = "heartbeatendpoint",
                sessionHostId = "serverid"
            };

            _mockHttpClient.Setup(x => x.SendHeartbeatAsync(It.IsAny<HeartbeatRequest>()))
                .ReturnsAsync(new HeartbeatResponse
                {
                    Operation = GameOperation.Terminate
                });


            var sdk = new InternalSdk(_mockSystemOperations.Object, _mockHttpClientFactory.Object);
            sdk.Start(false);
            Thread.Sleep(3000);

            sdk.State.Should().Be(GameState.Terminating);
        }


        [TestMethod]
        public void GameState_MaintDateReturned_CallbackInvoked()
        {
            _testConfiguration = new
            {
                heartbeatEndpoint = "heartbeatendpoint",
                sessionHostId = "serverid"
            };

            _mockHttpClient.Setup(x => x.SendHeartbeatAsync(It.IsAny<HeartbeatRequest>()))
                .ReturnsAsync(new HeartbeatResponse
                {
                    Operation = GameOperation.Continue,
                    NextScheduledMaintenanceUtc = "2018-11-12T04:11:14Z"
                });


            DateTimeOffset maintDate = DateTime.MinValue;
            ManualResetEvent evt = new ManualResetEvent(false);

            var sdk = new InternalSdk(_mockSystemOperations.Object, _mockHttpClientFactory.Object)
            {
                MaintenanceCallback = dt =>
                {
                    maintDate = dt;
                    evt.Set();
                }
            };


            sdk.Start(false);

            evt.WaitOne();
            maintDate.Should().Be(
                DateTime.Parse(
                    "2018-11-12T04:11:14Z",
                    null,
                    DateTimeStyles.RoundtripKind));


        }

        [TestMethod]
        public void GameState_MaintDateReturned_CallbackInvokedOnlyOnce()
        {
            _testConfiguration = new
            {
                heartbeatEndpoint = "heartbeatendpoint",
                sessionHostId = "serverid"
            };

            _mockHttpClient.Setup(x => x.SendHeartbeatAsync(It.IsAny<HeartbeatRequest>()))
                .ReturnsAsync(new HeartbeatResponse
                {
                    Operation = GameOperation.Continue,
                    NextScheduledMaintenanceUtc = "2018-11-12T04:11:14Z"
                });


            int invocationCount = 0;

            var sdk = new InternalSdk(_mockSystemOperations.Object, _mockHttpClientFactory.Object) {MaintenanceCallback = dt => { invocationCount++; }};

            sdk.Start(false);

            Thread.Sleep(5000);

            invocationCount.Should().Be(1);
        }

        [TestMethod]
        public void GameState_SessionConfigReturnedNoPlayers_ConfigMapUpdated()
        {
            _testConfiguration = new
            {
                heartbeatEndpoint = "heartbeatendpoint",
                sessionHostId = "serverid"
            };

            _mockHttpClient.Setup(x => x.SendHeartbeatAsync(It.IsAny<HeartbeatRequest>()))
                .ReturnsAsync(new HeartbeatResponse
                {
                    Operation = GameOperation.Continue,
                    SessionConfig = new SessionConfig
                    {
                        SessionId = Guid.NewGuid(),
                        SessionCookie = "awesomeCookie"
                    }
                });


            var sdk = new InternalSdk(_mockSystemOperations.Object, _mockHttpClientFactory.Object);

            sdk.Start(false);

            sdk.ConfigMap.Should().NotContainKey("sessionCookie");
            Thread.Sleep(2000);
            sdk.ConfigMap.Should().ContainKey("sessionCookie");
            sdk.InitialPlayers.Should().BeEmpty("Initial Player List not returned");

        }


        [TestMethod]
        public void GameState_SessionConfigReturnedWithPlayers_ConfigMapUpdated()
        {
            _testConfiguration = new
            {
                heartbeatEndpoint = "heartbeatendpoint",
                sessionHostId = "serverid"
            };

            List<string> playerList = new List<string>
            {
                "player1",
                "player2",
                "player3"
            };

            _mockHttpClient.Setup(x => x.SendHeartbeatAsync(It.IsAny<HeartbeatRequest>()))
                .ReturnsAsync(new HeartbeatResponse
                {
                    Operation = GameOperation.Continue,
                    SessionConfig = new SessionConfig
                    {
                        SessionId = Guid.NewGuid(),
                        SessionCookie = "awesomeCookie",
                        InitialPlayers = playerList
                    }
                });


            var sdk = new InternalSdk(_mockSystemOperations.Object, _mockHttpClientFactory.Object);

            sdk.Start(false);

            sdk.ConfigMap.Should().NotContainKey("sessionCookie");
            Thread.Sleep(2000);
            sdk.ConfigMap.Should().ContainKey("sessionCookie");
            sdk.InitialPlayers.Should().Contain(playerList, "Initial Player List returned");

        }
        
        [TestMethod]
        public void GameState_SessionConfigReturnedWithSessionMetadata_ConfigMapUpdated()
        {
            _testConfiguration = new
            {
                heartbeatEndpoint = "heartbeatendpoint",
                sessionHostId = "serverid"
            };

            var sessionMetadataKey = "sessionMetadataKey";
            var sessionMetadataValue = "sessionMetadataValue";
            _mockHttpClient.Setup(x => x.SendHeartbeatAsync(It.IsAny<HeartbeatRequest>()))
                .ReturnsAsync(new HeartbeatResponse
                {
                    Operation = GameOperation.Continue,
                    SessionConfig = new SessionConfig
                    {
                        SessionId = Guid.NewGuid(),
                        SessionCookie = "awesomeCookie",
                        Metadata = new Dictionary<string, string>() { { sessionMetadataKey, sessionMetadataValue } }
                    }
                });


            var sdk = new InternalSdk(_mockSystemOperations.Object, _mockHttpClientFactory.Object);

            sdk.Start(false);

            sdk.ConfigMap.Should().NotContainKey("sessionCookie");
            Thread.Sleep(2000);
            sdk.ConfigMap.Should().ContainKey("sessionCookie");
            Assert.AreEqual(sessionMetadataValue, sdk.ConfigMap[sessionMetadataKey]);

        }

        [TestMethod]
        public void GameState_TerminateReturned_CallbackInvoked()
        {
            _testConfiguration = new
            {
                heartbeatEndpoint = "heartbeatendpoint",
                sessionHostId = "serverid"
            };

            _mockHttpClient.Setup(x => x.SendHeartbeatAsync(It.IsAny<HeartbeatRequest>()))
                .ReturnsAsync(new HeartbeatResponse
                {
                    Operation = GameOperation.Terminate
                });


            bool shutdownInvoked = false;

            var sdk = new InternalSdk(_mockSystemOperations.Object, _mockHttpClientFactory.Object) {ShutdownCallback = () => { shutdownInvoked = true; }};


            sdk.Start(false);
            Thread.Sleep(2000);

            shutdownInvoked.Should().BeTrue();

        }

        [TestMethod]
        public void GameState_HealthCallbackReturnsUnhealthy_StatusSent()
        {
            HeartbeatRequest request = null;

            _testConfiguration = new
            {
                heartbeatEndpoint = "heartbeatendpoint",
                sessionHostId = "serverid"
            };

            _mockHttpClient.Setup(x => x.SendHeartbeatAsync(It.IsAny<HeartbeatRequest>()))
                .ReturnsAsync(new HeartbeatResponse
                {
                    Operation = GameOperation.Active
                })
                .Callback<HeartbeatRequest>(x => request = x);

            var sdk = new InternalSdk(_mockSystemOperations.Object, _mockHttpClientFactory.Object);

            sdk.HealthCallback = () => false;

            sdk.Start(false);

            sdk.State = GameState.StandingBy;
            sdk.TransitionToActiveEvent.WaitOne();

            request.Should().NotBeNull();
            request.CurrentGameHealth.Should().Be("Unhealthy");
        }

        [TestMethod]
        public void GameState_HealthCallbackReturnsHealthy_StatusSent()
        {
            HeartbeatRequest request = null;

            _testConfiguration = new
            {
                heartbeatEndpoint = "heartbeatendpoint",
                sessionHostId = "serverid"
            };

            _mockHttpClient.Setup(x => x.SendHeartbeatAsync(It.IsAny<HeartbeatRequest>()))
                .ReturnsAsync(new HeartbeatResponse
                {
                    Operation = GameOperation.Active
                })
                .Callback<HeartbeatRequest>(x => request = x);


            var sdk = new InternalSdk(_mockSystemOperations.Object, _mockHttpClientFactory.Object);

            sdk.HealthCallback = () => true;

            sdk.Start(false);

            sdk.State = GameState.StandingBy;
            sdk.TransitionToActiveEvent.WaitOne();

            request.Should().NotBeNull();
            request.CurrentGameHealth.Should().Be("Healthy");
        }
    }
}