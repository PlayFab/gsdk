using FluentAssertions;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace Microsoft.Playfab.Gaming.GSDK.CSharp.Test
{
    [TestClass]
    public class ConfigurationTests
    {
        [TestMethod]
        public async Task ReadConfiguration_MissingHeartbeatUrl_ShouldThrow()
        {
            var testConfig = new { };

            await ConfigFileWrapper.WrapAsync(testConfig, (fileName) =>
            {
                Action a = () => new JsonFileConfiguration(fileName);

                a.Should()
                    .Throw<GSDKInitializationException>()
                    .WithMessage($"*{fileName}*");

                return Task.CompletedTask;
            });
        }

        [TestMethod]
        public async Task ReadConfiguration_MissingServerId_ShouldThrow()
        {
            var testConfig = new { };

            await ConfigFileWrapper.WrapAsync(testConfig, (fileName) =>
            {
                Action a = () => new JsonFileConfiguration(fileName);
                a.Should()
                    .Throw<GSDKInitializationException>()
                    .WithMessage($"*{fileName}*");

                return Task.CompletedTask;
            });
        }

        [TestMethod]
        public async Task ReadConfiguration_AdditionalAttributeInFile_Ignored()
        {
            var testConfig = new
            {
                heartbeatEndpoint = "heartbeatendpoint",
                sessionHostId = "serverid",
                foo = "berry",
            };

            await ConfigFileWrapper.WrapAsync(testConfig, (fileName) =>
            {
                new JsonFileConfiguration(fileName);
                return Task.CompletedTask;
            });
        }

        [TestMethod]
        public async Task ReadConfiguration_EmptyGameCertificates_Parsed()
        {
            var testConfig = new
            {
                heartbeatEndpoint = "heartbeatendpoint",
                sessionHostId = "serverid",
                gameCertificates = new
                {
                }
            };

            await ConfigFileWrapper.WrapAsync(testConfig, (fileName) =>
            {
                Configuration c = new JsonFileConfiguration(fileName);
                c.GameCertificates.Should().NotBeNull();
                c.GameCertificates.Should().HaveCount(0);

                return Task.CompletedTask;
            });
        }

        [TestMethod]
        public async Task ReadConfiguration_MultipleGameCertificates_Parsed()
        {
            var testConfig = new
            {
                heartbeatEndpoint = "heartbeatendpoint",
                sessionHostId = "serverid",
                gameCertificates = new Dictionary<string, string>
                {
                    { "gameCert", "onetwothree" },
                    { "gameCert2", "threefourfive"},
                }
            };

            await ConfigFileWrapper.WrapAsync(testConfig, (fileName) =>
                    {
                        Configuration c = new JsonFileConfiguration(fileName);

                        c.GameCertificates.Should().NotBeNull();
                        c.GameCertificates.Should().HaveCount(2);

                        return Task.CompletedTask;
                    });
        }


        [TestMethod]
        public async Task ReadConfiguration_EmptyGameGamePorts_Parsed()
        {
            var testConfig = new
            {
                heartbeatEndpoint = "heartbeatendpoint",
                sessionHostId = "serverid",
                gamePorts = new
                {
                }
            };

            await ConfigFileWrapper.WrapAsync(testConfig, (fileName) =>
            {
                Configuration c = new JsonFileConfiguration(fileName);
                c.GamePorts.Should().NotBeNull();
                c.GamePorts.Should().HaveCount(0);

                return Task.CompletedTask;
            });
        }

        [TestMethod]
        public async Task ReadConfiguration_MultipleGamePorts_Parsed()
        {
            var testConfig = new
            {
                heartbeatEndpoint = "heartbeatendpoint",
                sessionHostId = "serverid",
                gamePorts = new Dictionary<string, string>
                {
                    { "8080", "debug" },
                    { "8081", "debug" },
                }
            };

            await ConfigFileWrapper.WrapAsync(testConfig, (fileName) =>
                    {
                        Configuration c = new JsonFileConfiguration(fileName);

                        c.GamePorts.Should().NotBeNull();
                        c.GamePorts.Should().HaveCount(2);

                        return Task.CompletedTask;
                    });
        }

        [TestMethod]
        public void ReadConfiguration_EnvConfigMissingReqProps_Throws()
        {
            Environment.SetEnvironmentVariable("HEARTBEAT_ENDPOINT", string.Empty);
            Environment.SetEnvironmentVariable("SESSION_HOST_ID", string.Empty);

            Action a = () => new EnvironmentVariableConfiguration();

            a.Should().Throw<GSDKInitializationException>()
                .WithMessage($"*Heartbeat*");
        }
    }
}
