namespace PlayFab.MultiplayerAgent.Model
{
    using Helpers;
    using System;

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