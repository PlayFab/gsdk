namespace Microsoft.Playfab.Gaming.GSDK.CSharp.Model
{
    using Newtonsoft.Json;
    using Newtonsoft.Json.Converters;

    internal class HeartbeatResponse
    {
        [JsonProperty(PropertyName = "sessionConfig")]
        public SessionConfig SessionConfig { get; set; }

        [JsonProperty(PropertyName = "nextScheduledMaintenanceUtc")]
        public string NextScheduledMaintenanceUtc { get; set; }

        [JsonProperty(PropertyName = "maintenanceSchedule")]
        public MaintenanceSchedule MaintenanceSchedule { get; set; }

        [JsonProperty(PropertyName = "operation", ItemConverterType = typeof(StringEnumConverter))]
        public GameOperation Operation { get; set; }
    }
}