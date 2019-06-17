namespace Microsoft.Playfab.Gaming.GSDK.CSharp.Test
{
    using System.Collections.Generic;
    using FluentAssertions;
    using Model;
    using Newtonsoft.Json;
    using VisualStudio.TestTools.UnitTesting;

    [TestClass]
    public class GSDKConfigurationTests
    {
        [TestMethod]
        public void ReadConfiguration_EmptyGameCertificates_Parsed()
        {
            var testConfig = new
            {
                heartbeatEndpoint = "heartbeatendpoint",
                sessionHostId = "serverid",
                gameCertificates = new
                {
                }
            };

            GSDKConfiguration c = JsonConvert.DeserializeObject<GSDKConfiguration>(JsonConvert.SerializeObject(testConfig));
            c.GameCertificates.Should().NotBeNull();
            c.GameCertificates.Should().HaveCount(0);
        }

        [TestMethod]
        public void ReadConfiguration_MultipleGameCertificates_Parsed()
        {
            var testConfig = new
            {
                heartbeatEndpoint = "heartbeatendpoint",
                sessionHostId = "serverid",
                gameCertificates = new Dictionary<string, string>
                {
                    {"gameCert", "onetwothree"},
                    {"gameCert2", "threefourfive"}
                }
            };
            GSDKConfiguration c = JsonConvert.DeserializeObject<GSDKConfiguration>(JsonConvert.SerializeObject(testConfig));

            c.GameCertificates.Should().NotBeNull();
            c.GameCertificates.Should().HaveCount(2);
        }


        [TestMethod]
        public void ReadConfiguration_EmptyGameGamePorts_Parsed()
        {
            var testConfig = new
            {
                heartbeatEndpoint = "heartbeatendpoint",
                sessionHostId = "serverid",
                gamePorts = new
                {
                }
            };

            GSDKConfiguration c = JsonConvert.DeserializeObject<GSDKConfiguration>(JsonConvert.SerializeObject(testConfig));
            c.GamePorts.Should().NotBeNull();
            c.GamePorts.Should().HaveCount(0);
        }

        [TestMethod]
        public void ReadConfiguration_MultipleGamePorts_Parsed()
        {
            var testConfig = new
            {
                heartbeatEndpoint = "heartbeatendpoint",
                sessionHostId = "serverid",
                gamePorts = new Dictionary<string, string>
                {
                    {"8080", "debug"},
                    {"8081", "debug"}
                }
            };

            GSDKConfiguration c = JsonConvert.DeserializeObject<GSDKConfiguration>(JsonConvert.SerializeObject(testConfig));

            c.GamePorts.Should().NotBeNull();
            c.GamePorts.Should().HaveCount(2);
        }
    }
}