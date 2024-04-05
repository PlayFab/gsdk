using System;
using System.Collections.Generic;
using PlayFab.MultiplayerAgent.Helpers;

namespace PlayFab.MultiplayerAgent.Model
{
    [Serializable]
    public class MaintenanceSchedule
    {
        [JsonProperty(PropertyName = "documentIncarnation")]
        public string DocumentIncarnation { get; set; }

        public List<MaintenanceEvent> Events { get; set; }
    }

    [Serializable]
    public class MaintenanceEvent
    {
        [JsonProperty(PropertyName = "eventId")]
        public string EventId { get; set; }
        
        [JsonProperty(PropertyName = "eventType")]
        public string EventType { get; set; }
        
        [JsonProperty(PropertyName = "resourceType")]
        public string ResourceType { get; set; }

        public List<string> Resources { get; set; }

        [JsonProperty(PropertyName = "eventStatus")]
        public string EventStatus { get; set; }
        
        [JsonProperty(PropertyName = "notBefore")]
        public DateTime? NotBefore { get; set; }
        
        [JsonProperty(PropertyName = "description")]
        public string Description { get; set; }
        
        [JsonProperty(PropertyName = "eventSource")]
        public string EventSource { get; set; }
        
        [JsonProperty(PropertyName = "durationInSeconds")]
        public int DurationInSeconds { get; set; }
    }
}
