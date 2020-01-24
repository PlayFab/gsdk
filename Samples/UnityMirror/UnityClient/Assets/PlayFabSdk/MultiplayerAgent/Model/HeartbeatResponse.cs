namespace PlayFab.MultiplayerAgent.Model
{
    using System;
    using PlayFab.Json;

    [Serializable]
    public class HeartbeatResponse
    {
        [JsonProperty(PropertyName = "sessionConfig")]
        public SessionConfig SessionConfig { get; set; }

        [JsonProperty(PropertyName = "nextScheduledMaintenanceUtc")]
        public string NextScheduledMaintenanceUtc { get; set; }

        [JsonProperty(PropertyName = "operation")]
        public GameOperation Operation { get; set; }
    }
}