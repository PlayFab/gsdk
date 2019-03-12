using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.IO;

namespace Microsoft.Playfab.Gaming.GSDK.CSharp
{
    internal class Configuration
    {
        public string HeartbeatEndpoint { get; protected set; }
        public string ServerId { get; protected set; }

        public string VmId { get; set; }
        public string LogFolder { get; protected set; }
        public string CertificateFolder { get; set; }

        /// <summary>
        /// A folder shared by all the game servers within a VM (to cache user generated content and other data).
        /// </summary>
        public string SharedContentFolder { get; set; }

        public string PublicIpV4Address { get; set; }

        public string FullyQualifiedDomainName { get; set; }

        public IDictionary<string, string> GameCertificates { get; set; }
        public string TitleId => Environment.GetEnvironmentVariable(TITLE_ID_ENV_VAR);
        public string BuildId => Environment.GetEnvironmentVariable(BUILD_ID_ENV_VAR);
        public string Region => Environment.GetEnvironmentVariable(REGION_ENV_VAR);
        public IDictionary<string, string> BuildMetadata { get; set; }
        public IDictionary<string, string> GamePorts { get; set; }
        
        private const string TITLE_ID_ENV_VAR = "PF_TITLE_ID";
        private const string BUILD_ID_ENV_VAR = "PF_BUILD_ID";
        private const string REGION_ENV_VAR = "PF_REGION";

        protected Configuration()
        {
            GameCertificates = new Dictionary<string, string>();
            BuildMetadata = new Dictionary<string, string>();
            GamePorts = new Dictionary<string, string>();
        }
    }

    internal class EnvironmentVariableConfiguration : Configuration
    {
        private const string HEARTBEAT_ENDPOINT_ENV_VAR = "HEARTBEAT_ENDPOINT";
        private const string SERVER_ID_ENV_VAR = "SESSION_HOST_ID";
        private const string LOG_FOLDER_ENV_VAR = "GSDK_LOG_FOLDER";
        private const string SHARED_CONTENT_FOLDER_ENV_VAR = "SHARED_CONTENT_FOLDER";
        
        public EnvironmentVariableConfiguration() : base()
        {
            HeartbeatEndpoint = Environment.GetEnvironmentVariable(HEARTBEAT_ENDPOINT_ENV_VAR);
            ServerId = Environment.GetEnvironmentVariable(SERVER_ID_ENV_VAR);
            LogFolder = Environment.GetEnvironmentVariable(LOG_FOLDER_ENV_VAR);
            SharedContentFolder = Environment.GetEnvironmentVariable(SHARED_CONTENT_FOLDER_ENV_VAR);
            if (string.IsNullOrWhiteSpace(HeartbeatEndpoint) || string.IsNullOrWhiteSpace(ServerId))
            {
                throw new GSDKInitializationException("Heartbeat endpoint and Server id are required configuration values.");
            }
        }
    }

    internal class JsonFileConfiguration : Configuration
    {
        public JsonFileConfiguration(string fileName) : base()
        {
            try
            {
                using (StreamReader reader = File.OpenText(fileName))
                {
                    JsonSchema config = JsonConvert.DeserializeObject<JsonSchema>(reader.ReadToEnd());

                    HeartbeatEndpoint = config.HeartbeatEndpoint;
                    ServerId = config.SessionHostId;
                    VmId = config.VmId;
                    LogFolder = config.LogFolder;
                    SharedContentFolder = config.SharedContentFolder;
                    CertificateFolder = config.CertificateFolder;
                    GameCertificates = config.GameCertificates ?? new Dictionary<string, string>();
                    GamePorts = config.GamePorts ?? new Dictionary<string, string>();
                    BuildMetadata = config.BuildMetadata ?? new Dictionary<string, string>();
                    PublicIpV4Address = config.PublicIpV4Address;
                    FullyQualifiedDomainName = config.FullyQualifiedDomainName;
                }
            }
            catch (Exception ex)
            {
                throw new GSDKInitializationException($"Cannot read configuration file {fileName}", ex);
            }
        }
        
        private class JsonSchema
        {
            [JsonProperty(PropertyName = "heartbeatEndpoint", Required = Required.Always)]
            public string HeartbeatEndpoint { get; set; }

            [JsonProperty(PropertyName = "sessionHostId", Required = Required.Always)]
            public string SessionHostId { get; set; }

            [JsonProperty(PropertyName = "vmId", Required = Required.Always)]
            public string VmId { get; set; }

            [JsonProperty(PropertyName = "logFolder", Required = Required.Default)]
            public string LogFolder { get; set; }

            [JsonProperty(PropertyName = "sharedContentFolder", Required = Required.Default)]
            public string SharedContentFolder { get; set; }

            [JsonProperty(PropertyName = "certificateFolder", Required = Required.Default)]
            public string CertificateFolder { get; set; }

            [JsonProperty(PropertyName = "gameCertificates", Required = Required.Default)]
            public IDictionary<string, string> GameCertificates { get; set; }

            [JsonProperty(PropertyName = "buildMetadata", Required = Required.Default)]
            public IDictionary<string, string> BuildMetadata { get; set; }

            [JsonProperty(PropertyName = "gamePorts", Required = Required.Default)]
            public IDictionary<string, string> GamePorts { get; set; }

            [JsonProperty(PropertyName = "publicIpV4Address", Required = Required.Always)]
            public string PublicIpV4Address { get; set; }

            [JsonProperty(PropertyName = "fullyQualifiedDomainName", Required = Required.Always)]
            public string FullyQualifiedDomainName { get; set; }
        }
    }
}