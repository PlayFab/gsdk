using System;
using System.Collections.Generic;
using System.Globalization;
using System.Threading;
using System.Threading.Tasks;
using FluentAssertions;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Moq;

namespace Microsoft.Playfab.Gaming.GSDK.CSharp.Test
{
    [TestClass]
    public class InternalSdkTests
    {
        internal Mock<IHttpClient> _clientMock;

        [TestInitialize]
        public void TestInitialize()
        {
            _clientMock = new Mock<IHttpClient>();

            HttpClientFactory.Instance = _clientMock.Object;

            _clientMock.Setup(x => x.SendHeartbeatAsync(It.IsAny<HeartbeatRequest>()))
                .Returns(Task.FromResult(new HeartbeatResponse()));
        }

        [TestCleanup]
        public void TestCleanup()
        {
            HttpClientFactory.Instance = null;
        }


        [TestMethod]
        public async Task Start_InvalidConfiguration_Throws()
        {
            var testConfig = new { ShouldLog = false };

            await ConfigFileWrapper.WrapAsync(testConfig, (fileName) =>
            {
                Func<Task> a = async () =>
                {
                    var sdk = new InternalSdk(fileName);
                    await sdk.StartAsync(false);
                };

                a.Should()
                    .Throw<GSDKInitializationException>()
                    .WithMessage($"*{fileName}*");

                return Task.FromResult(0);
            });
        }

        [TestMethod]
        public async Task Start_ValidConfiguration_Starts()
        {
            var testConfig = new
            {
                ShouldLog = false,
                heartbeatEndpoint = "heartbeatendpoint",
                sessionHostId = "serverid",
                gameCertificates = new Dictionary<string, string>
                {
                    { "cert1", "thumb1" },
                    { "cert2", "thumb2" },
                    { "cert3", "thumb3" },
                },
                buildMetadata = new Dictionary<string, string>
                {
                    { "property1", "value1" },
                    { "property2", "value2" },
                    { "property3", "value3" },
                },
                gamePorts = new Dictionary<string, string>
                {
                    { "8080", "port1" },
                }
            };

            await ConfigFileWrapper.WrapAsync(testConfig, async (fileName) =>
            {
                using (var sdk = new InternalSdk(fileName))
                {
                    await sdk.StartAsync(false);

                    sdk.State.Should().Be(GameState.Invalid);
                    sdk.ConfigMap.Should().NotBeNull();
                    sdk.ConfigMap.Should().HaveCountGreaterThan(0);
                    sdk.ConnectedPlayers.Should().NotBeNull();
                    sdk.ConnectedPlayers.Should().HaveCount(0);
                }
            });
        }

        [TestMethod]
        public async Task Start_DisposeShouldFire_HeartbeatStopped()
        {
            var testConfig = new
            {
                ShouldLog = false,
                heartbeatEndpoint = "heartbeatendpoint",
                sessionHostId = "serverid",
            };

            await ConfigFileWrapper.WrapAsync(testConfig, async (fileName) =>
            {
                var sdk = new Mock<InternalSdk>(fileName);

                try
                {
                    await sdk.Object.StartAsync(true);
                    Thread.Sleep(1000);
                }
                finally
                {
                    sdk.Object.Dispose();
                    Thread.Sleep(3000);
                    sdk.Verify(x => x.SendHeartbeatAsync(), Times.AtMost(2));
                }
            });
        }

        [TestMethod]
        public async Task Start_RunsForIterations_HeartbeatCountCorrect()
        {
            var testConfig = new
            {
                ShouldLog = false,
                heartbeatEndpoint = "heartbeatendpoint",
                sessionHostId = "serverid",
            };

            await ConfigFileWrapper.WrapAsync(testConfig, async (fileName) =>
            {
                var sdk = new Mock<InternalSdk>(fileName);

                try
                {
                    await sdk.Object.StartAsync(true);
                    Thread.Sleep(5000);
                }
                finally
                {
                    sdk.Object.Dispose();
                    sdk.Verify(x => x.SendHeartbeatAsync(), Times.AtLeast(4));
                    sdk.Verify(x => x.SendHeartbeatAsync(), Times.AtMost(5));
                }
            });
        }

        [TestMethod]
        public async Task GameStateToActive_ActiveReturned_StateUpdated()
        {
            var testConfig = new
            {
                ShouldLog = false,
                heartbeatEndpoint = "heartbeatendpoint",
                sessionHostId = "serverid",
            };

            _clientMock.Setup(x => x.SendHeartbeatAsync(It.IsAny<HeartbeatRequest>()))
                .ReturnsAsync(new HeartbeatResponse
                {
                    Operation = GameOperation.Active
                });

            await ConfigFileWrapper.WrapAsync(testConfig, async (fileName) =>
            {
                using (var sdk = new InternalSdk(fileName))
                {
                    await sdk.StartAsync(false);

                    sdk.State = GameState.StandingBy;
                    sdk.TransitionToActiveEvent.WaitOne();
                    sdk.State.Should().Be(GameState.Active);
                }
            });
        }

        [TestMethod]
        public async Task GameState_TerminateReturned_StateUpdated()
        {
            var testConfig = new
            {
                ShouldLog = false,
                heartbeatEndpoint = "heartbeatendpoint",
                sessionHostId = "serverid",
            };

            _clientMock.Setup(x => x.SendHeartbeatAsync(It.IsAny<HeartbeatRequest>()))
                .ReturnsAsync(new HeartbeatResponse
                {
                    Operation = GameOperation.Terminate
                });

            await ConfigFileWrapper.WrapAsync(testConfig, async (fileName) =>
            {
                using (var sdk = new InternalSdk(fileName))
                {
                    await sdk.StartAsync(false);
                    Thread.Sleep(2000);

                    sdk.State.Should().Be(GameState.Terminating);
                }
            });
        }

        [TestMethod]
        public async Task GameState_MaintDateReturned_CallbackInvoked()
        {
            var testConfig = new
            {
                ShouldLog = false,
                heartbeatEndpoint = "heartbeatendpoint",
                sessionHostId = "serverid",
            };

            _clientMock.Setup(x => x.SendHeartbeatAsync(It.IsAny<HeartbeatRequest>()))
                .ReturnsAsync(new HeartbeatResponse
                {
                    Operation = GameOperation.Continue,
                    NextScheduledMaintenanceUtc = "2018-11-12T04:11:14Z",
                });

            await ConfigFileWrapper.WrapAsync(testConfig, async (fileName) =>
            {
                DateTimeOffset maintDate = DateTime.MinValue;
                var evt = new ManualResetEvent(false);

                using (var sdk = new InternalSdk(fileName))
                {
                    sdk.MaintenanceCallback = (dt) =>
                    {
                        maintDate = dt;
                        evt.Set();
                    };

                    await sdk.StartAsync(false);

                    evt.WaitOne();
                    maintDate.Should().Be(
                        DateTime.Parse(
                            "2018-11-12T04:11:14Z",
                            null,
                            DateTimeStyles.RoundtripKind));
                }
            });
        }

        [TestMethod]
        public async Task GameState_MaintDateReturned_CallbackInvokedOnlyOnce()
        {
            var testConfig = new
            {
                ShouldLog = false,
                heartbeatEndpoint = "heartbeatendpoint",
                sessionHostId = "serverid",
            };

            _clientMock.Setup(x => x.SendHeartbeatAsync(It.IsAny<HeartbeatRequest>()))
                .ReturnsAsync(new HeartbeatResponse
                {
                    Operation = GameOperation.Continue,
                    NextScheduledMaintenanceUtc = "2018-11-12T04:11:14Z",
                });

            await ConfigFileWrapper.WrapAsync(testConfig, async (fileName) =>
            {
                int invocationCount = 0;

                using (var sdk = new InternalSdk(fileName))
                {
                    sdk.MaintenanceCallback = (dt) =>
                    {
                        invocationCount++;
                    };

                    await sdk.StartAsync(false);

                    Thread.Sleep(5000);

                    invocationCount.Should().Be(1);
                }
            });
        }

        [TestMethod]
        public async Task GameState_SessionConfigReturnedNoPlayers_ConfigMapUpdated()
        {
            var testConfig = new
            {
                ShouldLog = false,
                heartbeatEndpoint = "heartbeatendpoint",
                sessionHostId = "serverid",
            };

            _clientMock.Setup(x => x.SendHeartbeatAsync(It.IsAny<HeartbeatRequest>()))
                .ReturnsAsync(new HeartbeatResponse
                {
                    Operation = GameOperation.Continue,
                    SessionConfig = new SessionConfig()
                    {
                        SessionId = Guid.NewGuid(),
                        SessionCookie = "awesomeCookie"
                    }
                });

            await ConfigFileWrapper.WrapAsync(testConfig, async (fileName) =>
            {
                using (var sdk = new InternalSdk(fileName))
                {
                    await sdk.StartAsync(false);

                    sdk.ConfigMap.Should().NotContainKey("sessionCookie");
                    Thread.Sleep(2000);
                    sdk.ConfigMap.Should().ContainKey("sessionCookie");
                    sdk.InitialPlayers.Should().BeEmpty("Initial Player List not returned");
                }
            });
        }

        [TestMethod]
        public async Task GameState_SessionConfigReturnedWithPlayers_ConfigMapUpdated()
        {
            var testConfig = new
            {
                ShouldLog = false,
                heartbeatEndpoint = "heartbeatendpoint",
                sessionHostId = "serverid",
            };

            List<string> playerList = new List<string>
            {
                "player1", "player2", "player3"
            };

            _clientMock.Setup(x => x.SendHeartbeatAsync(It.IsAny<HeartbeatRequest>()))
                .ReturnsAsync(new HeartbeatResponse
                {
                    Operation = GameOperation.Continue,
                    SessionConfig = new SessionConfig()
                    {
                        SessionId = Guid.NewGuid(),
                        SessionCookie = "awesomeCookie",
                        InitialPlayers = playerList
                    }
                });

            await ConfigFileWrapper.WrapAsync(testConfig, async (fileName) =>
            {
                using (var sdk = new InternalSdk(fileName))
                {
                    await sdk.StartAsync(false);

                    sdk.ConfigMap.Should().NotContainKey("sessionCookie");
                    Thread.Sleep(2000);
                    sdk.ConfigMap.Should().ContainKey("sessionCookie");
                    sdk.InitialPlayers.Should().Contain(playerList, "Initial Player List returned");
                }
            });
        }

        [TestMethod]
        public async Task GameState_TerminateReturned_CallbackInvoked()
        {
            var testConfig = new
            {
                ShouldLog = false,
                heartbeatEndpoint = "heartbeatendpoint",
                sessionHostId = "serverid",
            };

            _clientMock.Setup(x => x.SendHeartbeatAsync(It.IsAny<HeartbeatRequest>()))
                .ReturnsAsync(new HeartbeatResponse
                {
                    Operation = GameOperation.Terminate
                });

            await ConfigFileWrapper.WrapAsync(testConfig, async (fileName) =>
            {
                bool shutdownInvoked = false;

                using (var sdk = new InternalSdk(fileName))
                {
                    sdk.ShutdownCallback = () => { shutdownInvoked = true; };

                    await sdk.StartAsync(false);
                    Thread.Sleep(2000);

                    shutdownInvoked.Should().BeTrue();
                }
            });
        }

        [TestMethod]
        public async Task GameState_HealthCallbackReturnsUnhealthy_StatusSent()
        {
            HeartbeatRequest request = null;

            var testConfig = new
            {
                ShouldLog = false,
                heartbeatEndpoint = "heartbeatendpoint",
                sessionHostId = "serverid",
            };

            _clientMock.Setup(x => x.SendHeartbeatAsync(It.IsAny<HeartbeatRequest>()))
                .ReturnsAsync(new HeartbeatResponse
                {
                    Operation = GameOperation.Active,
                })
                .Callback<HeartbeatRequest>(x => request = x);

            await ConfigFileWrapper.WrapAsync(testConfig, async (fileName) =>
            {
                using (var sdk = new InternalSdk(fileName))
                {
                    sdk.HealthCallback = () => { return false; };

                    await sdk.StartAsync(false);

                    sdk.State = GameState.StandingBy;
                    sdk.TransitionToActiveEvent.WaitOne();

                    request.Should().NotBeNull();
                    request.CurrentGameHealth.Should().Be("Unhealthy");
                }
            });
        }

        [TestMethod]
        public async Task GameState_HealthCallbackReturnsHealthy_StatusSent()
        {
            HeartbeatRequest request = null;

            var testConfig = new
            {
                ShouldLog = false,
                heartbeatEndpoint = "heartbeatendpoint",
                sessionHostId = "serverid",
            };

            _clientMock.Setup(x => x.SendHeartbeatAsync(It.IsAny<HeartbeatRequest>()))
                .ReturnsAsync(new HeartbeatResponse
                {
                    Operation = GameOperation.Active,
                })
                .Callback<HeartbeatRequest>(x => request = x);

            await ConfigFileWrapper.WrapAsync(testConfig, async (fileName) =>
            {
                using (var sdk = new InternalSdk(fileName))
                {
                    sdk.HealthCallback = () => { return true; };

                    await sdk.StartAsync(false);

                    sdk.State = GameState.StandingBy;
                    sdk.TransitionToActiveEvent.WaitOne();

                    request.Should().NotBeNull();
                    request.CurrentGameHealth.Should().Be("Healthy");
                }
            });
        }
    }
}