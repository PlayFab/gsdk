namespace Microsoft.Playfab.Gaming.GSDK.CSharp.Model
{
    using System;
    using System.Collections.Generic;
    using Newtonsoft.Json;

    internal class SessionConfig
    {
        [JsonProperty(PropertyName = "sessionId")]
        public Guid SessionId { get; set; }

        [JsonProperty(PropertyName = "sessionCookie")]
        public string SessionCookie { get; set; }

        [JsonProperty(PropertyName = "initialPlayers")]
        public List<string> InitialPlayers { get; set; }
    }
}