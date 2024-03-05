using System;
using System.Collections.Generic;

namespace PlayFab.MultiplayerAgent.Model
{
    [Serializable]
    public class MaintenanceSchedule
    {
        public string DocumentIncarnation { get; set; }

        public List<MaintenanceEvent> Events { get; set; }
    }

    [Serializable]
    public class MaintenanceEvent
    {
        public string EventId { get; set; }
        
        public string EventType { get; set; }
        
        public string ResourceType { get; set; }

        public List<string> Resources { get; set; }

        public string EventStatus { get; set; }
        
        public DateTime? NotBefore { get; set; }

        public string Description { get; set; }

        public string EventSource { get; set; }
        
        public int DurationInSeconds { get; set; }
    }
}
