using Newtonsoft.Json;
using System;
using System.Collections.Generic;

namespace Microsoft.Playfab.Gaming.GSDK.CSharp.Model
{
    public class MaintenanceSchedule
    {
        public string DocumentIncarnation { get; set; }

        public IList<MaintenanceEvent> Events { get; set; }
    }

    public class MaintenanceEvent
    {
        public string EventId { get; set; }

        public string EventType { get; set; }

        public string ResourceType { get; set; }

        public IList<string> Resources { get; set; }

        public string EventStatus { get; set; }

        public DateTime? NotBefore { get; set; }

        public string Description { get; set; }

        public string EventSource { get; set; }

        public int DurationInSeconds { get; set; }
    }
}
