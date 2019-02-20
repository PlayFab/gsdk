using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.IO;

namespace Microsoft.Playfab.Gaming.GSDK.CSharp
{
    class Configuration
    {
        public string HeartbeatEndpoint { get; protected set; }
        public string ServerId { get; protected set; }
        public string LogFolder { get; protected set; }
        public string CertificateFolder { get; set; }

        /// <summary>
        /// A folder shared by all the game servers within a VM (to cache user generated content and other data).
        /// </summary>
        public string SharedContentFolder { get; set; }

        public IDictionary<string, string> GameCertificates { get; set; }
        public string TitleId { get; set; }
        public string BuildId { get; set; }
        public string Region { get; set; }
        public IDictionary<string, string> BuildMetadata { get; set; }
        public IDictionary<string, string> GamePorts { get; set; }

        protected const string HEARTBEAT_ENDPOINT_ENV_VAR = "HEARTBEAT_ENDPOINT";
        protected const string SERVER_ID_ENV_VAR = "SESSION_HOST_ID";
        protected const string LOG_FOLDER_ENV_VAR = "GSDK_LOG_FOLDER";
        protected const string TITLE_ID_ENV_VAR = "PF_TITLE_ID";
        protected const string BUILD_ID_ENV_VAR = "PF_BUILD_ID";
        protected const string REGION_ENV_VAR = "PF_REGION";
        protected const string SHARED_CONTENT_FOLDER_ENV_VAR = "SHARED_CONTENT_FOLDER";

        public Configuration()
        {
            GameCertificates = new Dictionary<string, string>();
            BuildMetadata = new Dictionary<string, string>();
            GamePorts = new Dictionary<string, string>();
        }
    }

    class EnvironmentVariableConfiguration : Configuration
    {
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

    class JsonFileConfiguration : Configuration
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
                    LogFolder = config.LogFolder;
                    SharedContentFolder = config.SharedContentFolder;
                    CertificateFolder = config.CertificateFolder;
                    GameCertificates = config.GameCertificates ?? new Dictionary<string, string>();
                    GamePorts = config.GamePorts ?? new Dictionary<string, string>();
                    BuildMetadata = config.BuildMetadata ?? new Dictionary<string, string>();
                }
            }
            catch (Exception ex)
            {
                throw new GSDKInitializationException($"Cannot read configuration file {fileName}", ex);
            }
        }
    }

    class JsonSchema
    {
        [JsonProperty(PropertyName = "heartbeatEndpoint", Required = Required.Always)]
        public string HeartbeatEndpoint { get; set; }

        [JsonProperty(PropertyName = "sessionHostId", Required = Required.Always)]
        public string SessionHostId { get; set; }

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
    }
}