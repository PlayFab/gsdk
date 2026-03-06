namespace PlayFab.MultiplayerAgent.Tests
{
    using System;
    using System.Collections.Generic;
    using Helpers;
    using Model;
    using Microsoft.VisualStudio.TestTools.UnitTesting;

    [TestClass]
    public class GSDKConfigurationTests
    {
        private static readonly SimpleJsonInstance _jsonInstance = new SimpleJsonInstance();

        [TestMethod]
        public void ReadConfiguration_EmptyGameCertificates_Parsed()
        {
            string json = @"
            {
                ""heartbeatEndpoint"": ""heartbeatendpoint"",
                ""sessionHostId"": ""serverid"",
                ""gameCertificates"": {}
            }";

            GSDKConfiguration config = _jsonInstance.DeserializeObject<GSDKConfiguration>(json);
            Assert.IsNotNull(config.GameCertificates);
            Assert.AreEqual(0, config.GameCertificates.Count);
        }

        [TestMethod]
        public void ReadConfiguration_MultipleGameCertificates_Parsed()
        {
            string json = @"
            {
                ""heartbeatEndpoint"": ""heartbeatendpoint"",
                ""sessionHostId"": ""serverid"",
                ""gameCertificates"": {
                    ""gameCert"": ""onetwothree"",
                    ""gameCert2"": ""threefourfive""
                }
            }";

            GSDKConfiguration config = _jsonInstance.DeserializeObject<GSDKConfiguration>(json);
            Assert.IsNotNull(config.GameCertificates);
            Assert.AreEqual(2, config.GameCertificates.Count);
            Assert.AreEqual("onetwothree", config.GameCertificates["gameCert"]);
            Assert.AreEqual("threefourfive", config.GameCertificates["gameCert2"]);
        }

        [TestMethod]
        public void ReadConfiguration_EmptyGamePorts_Parsed()
        {
            string json = @"
            {
                ""heartbeatEndpoint"": ""heartbeatendpoint"",
                ""sessionHostId"": ""serverid"",
                ""gamePorts"": {}
            }";

            GSDKConfiguration config = _jsonInstance.DeserializeObject<GSDKConfiguration>(json);
            Assert.IsNotNull(config.GamePorts);
            Assert.AreEqual(0, config.GamePorts.Count);
        }

        [TestMethod]
        public void ReadConfiguration_MultipleGamePorts_Parsed()
        {
            string json = @"
            {
                ""heartbeatEndpoint"": ""heartbeatendpoint"",
                ""sessionHostId"": ""serverid"",
                ""gamePorts"": {
                    ""8080"": ""debug"",
                    ""8081"": ""game""
                }
            }";

            GSDKConfiguration config = _jsonInstance.DeserializeObject<GSDKConfiguration>(json);
            Assert.IsNotNull(config.GamePorts);
            Assert.AreEqual(2, config.GamePorts.Count);
            Assert.AreEqual("debug", config.GamePorts["8080"]);
            Assert.AreEqual("game", config.GamePorts["8081"]);
        }

        [TestMethod]
        public void ReadConfiguration_BuildMetadata_Parsed()
        {
            string json = @"
            {
                ""heartbeatEndpoint"": ""heartbeatendpoint"",
                ""sessionHostId"": ""serverid"",
                ""buildMetadata"": {
                    ""key1"": ""value1"",
                    ""key2"": ""value2""
                }
            }";

            GSDKConfiguration config = _jsonInstance.DeserializeObject<GSDKConfiguration>(json);
            Assert.IsNotNull(config.BuildMetadata);
            Assert.AreEqual(2, config.BuildMetadata.Count);
            Assert.AreEqual("value1", config.BuildMetadata["key1"]);
            Assert.AreEqual("value2", config.BuildMetadata["key2"]);
        }

        [TestMethod]
        public void ReadConfiguration_AllFieldsSet_Parsed()
        {
            string json = @"
            {
                ""heartbeatEndpoint"": ""testEndpoint"",
                ""sessionHostId"": ""testServerId"",
                ""vmId"": ""testVmId"",
                ""logFolder"": ""testLogFolder"",
                ""sharedContentFolder"": ""testSharedContentFolder"",
                ""certificateFolder"": ""testCertFolder"",
                ""publicIpV4Address"": ""1.2.3.4"",
                ""fullyQualifiedDomainName"": ""test.example.com"",
                ""gameCertificates"": { ""cert1"": ""thumbprint1"" },
                ""buildMetadata"": { ""key1"": ""value1"" },
                ""gamePorts"": { ""port1"": ""1111"" }
            }";

            GSDKConfiguration config = _jsonInstance.DeserializeObject<GSDKConfiguration>(json);
            Assert.AreEqual("testEndpoint", config.HeartbeatEndpoint);
            Assert.AreEqual("testServerId", config.SessionHostId);
            Assert.AreEqual("testVmId", config.VmId);
            Assert.AreEqual("testLogFolder", config.LogFolder);
            Assert.AreEqual("testSharedContentFolder", config.SharedContentFolder);
            Assert.AreEqual("testCertFolder", config.CertificateFolder);
            Assert.AreEqual("1.2.3.4", config.PublicIpV4Address);
            Assert.AreEqual("test.example.com", config.FullyQualifiedDomainName);
            Assert.AreEqual("thumbprint1", config.GameCertificates["cert1"]);
            Assert.AreEqual("value1", config.BuildMetadata["key1"]);
            Assert.AreEqual("1111", config.GamePorts["port1"]);
        }

        [TestMethod]
        public void ReadConfiguration_EnvironmentVariables_Captured()
        {
            Environment.SetEnvironmentVariable("PF_TITLE_ID", "testTitleId");
            Environment.SetEnvironmentVariable("PF_BUILD_ID", "testBuildId");
            Environment.SetEnvironmentVariable("PF_REGION", "testRegion");

            try
            {
                var config = new GSDKConfiguration();
                Assert.AreEqual("testTitleId", config.TitleId);
                Assert.AreEqual("testBuildId", config.BuildId);
                Assert.AreEqual("testRegion", config.Region);
            }
            finally
            {
                Environment.SetEnvironmentVariable("PF_TITLE_ID", null);
                Environment.SetEnvironmentVariable("PF_BUILD_ID", null);
                Environment.SetEnvironmentVariable("PF_REGION", null);
            }
        }

        [TestMethod]
        public void ReadConfiguration_GameServerConnectionInfo_Parsed()
        {
            string json = @"
            {
                ""heartbeatEndpoint"": ""heartbeatendpoint"",
                ""sessionHostId"": ""serverid"",
                ""gameServerConnectionInfo"": {
                    ""publicIpV4Address"": ""10.0.0.1"",
                    ""gamePortsConfiguration"": [
                        {
                            ""name"": ""game"",
                            ""serverListeningPort"": 7777,
                            ""clientConnectionPort"": 30000
                        }
                    ]
                }
            }";

            GSDKConfiguration config = _jsonInstance.DeserializeObject<GSDKConfiguration>(json);
            Assert.IsNotNull(config.GameServerConnectionInfo);
            Assert.AreEqual("10.0.0.1", config.GameServerConnectionInfo.PublicIPv4Address);
        }

        [TestMethod]
        public void ReadConfiguration_OptionalFieldsNull_DefaultsUsed()
        {
            string json = @"
            {
                ""heartbeatEndpoint"": ""testEndpoint"",
                ""sessionHostId"": ""testServerId""
            }";

            GSDKConfiguration config = _jsonInstance.DeserializeObject<GSDKConfiguration>(json);
            Assert.AreEqual("testEndpoint", config.HeartbeatEndpoint);
            Assert.AreEqual("testServerId", config.SessionHostId);
            Assert.IsNull(config.LogFolder);
            Assert.IsNull(config.SharedContentFolder);
            Assert.IsNull(config.CertificateFolder);
        }

        [TestMethod]
        public void SerializeAndDeserialize_RoundTrip_PreservesData()
        {
            var config = new GSDKConfiguration
            {
                HeartbeatEndpoint = "testEndpoint",
                SessionHostId = "testServerId",
                LogFolder = "testLogFolder"
            };
            config.GameCertificates["cert1"] = "thumbprint1";
            config.GamePorts["port1"] = "1111";
            config.BuildMetadata["key1"] = "value1";

            string json = _jsonInstance.SerializeObject(config);
            GSDKConfiguration deserialized = _jsonInstance.DeserializeObject<GSDKConfiguration>(json);

            Assert.AreEqual(config.HeartbeatEndpoint, deserialized.HeartbeatEndpoint);
            Assert.AreEqual(config.SessionHostId, deserialized.SessionHostId);
            Assert.AreEqual(config.LogFolder, deserialized.LogFolder);
            Assert.AreEqual("thumbprint1", deserialized.GameCertificates["cert1"]);
            Assert.AreEqual("1111", deserialized.GamePorts["port1"]);
            Assert.AreEqual("value1", deserialized.BuildMetadata["key1"]);
        }
    }
}
