using FluentAssertions;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace Microsoft.Playfab.Gaming.GSDK.CSharp.Test
{
    using System.IO;
    using Newtonsoft.Json;

    [TestClass]
    public class ConfigurationTests
    {
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
                JsonConvert.DeserializeObject<GsdkConfiguration>(File.ReadAllText(fileName));
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
                GsdkConfiguration c = JsonConvert.DeserializeObject<GsdkConfiguration>(File.ReadAllText(fileName));
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
                        GsdkConfiguration c = JsonConvert.DeserializeObject<GsdkConfiguration>(File.ReadAllText(fileName));

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
                GsdkConfiguration c = JsonConvert.DeserializeObject<GsdkConfiguration>(File.ReadAllText(fileName));
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
                        GsdkConfiguration c = JsonConvert.DeserializeObject<GsdkConfiguration>(File.ReadAllText(fileName));

                        c.GamePorts.Should().NotBeNull();
                        c.GamePorts.Should().HaveCount(2);

                        return Task.CompletedTask;
                    });
        }
    }
}
