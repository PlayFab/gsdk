namespace Microsoft.Playfab.Gaming.GSDK.CSharp
{
    using System;
    using System.Collections.Generic;
    using Newtonsoft.Json;

    internal class GsdkConfiguration
    {
        private const string TITLE_ID_ENV_VAR = "PF_TITLE_ID";
        private const string BUILD_ID_ENV_VAR = "PF_BUILD_ID";
        private const string REGION_ENV_VAR = "PF_REGION";

        public GsdkConfiguration()
        {
            GameCertificates = new Dictionary<string, string>();
            BuildMetadata = new Dictionary<string, string>();
            GamePorts = new Dictionary<string, string>();
        }

        public string TitleId => Environment.GetEnvironmentVariable(TITLE_ID_ENV_VAR);
        public string BuildId => Environment.GetEnvironmentVariable(BUILD_ID_ENV_VAR);
        public string Region => Environment.GetEnvironmentVariable(REGION_ENV_VAR);

        [JsonProperty(PropertyName = "heartbeatEndpoint", Required = Required.Always)]
        public string HeartbeatEndpoint { get; set; }

        [JsonProperty(PropertyName = "sessionHostId", Required = Required.Always)]
        public string SessionHostId { get; set; }

        [JsonProperty(PropertyName = "vmId")]
        public string VmId { get; set; }

        [JsonProperty(PropertyName = "logFolder")]
        public string LogFolder { get; set; }

        [JsonProperty(PropertyName = "sharedContentFolder")]
        public string SharedContentFolder { get; set; }

        [JsonProperty(PropertyName = "certificateFolder")]
        public string CertificateFolder { get; set; }

        [JsonProperty(PropertyName = "gameCertificates")]
        public IDictionary<string, string> GameCertificates { get; set; }

        [JsonProperty(PropertyName = "buildMetadata")]
        public IDictionary<string, string> BuildMetadata { get; set; }

        [JsonProperty(PropertyName = "gamePorts")]
        public IDictionary<string, string> GamePorts { get; set; }

        [JsonProperty(PropertyName = "publicIpV4Address")]
        public string PublicIpV4Address { get; set; }

        [JsonProperty(PropertyName = "fullyQualifiedDomainName")]
        public string FullyQualifiedDomainName { get; set; }

        [JsonProperty(PropertyName = "gameServerConnectionInfo")]
        public GameServerConnectionInfo GameServerConnectionInfo { get; set; }
    }
}