namespace PlayFab.MultiplayerAgent.Model
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using PlayFab.Json;

    [Serializable]
    public class SessionConfig
    {
        [JsonProperty(PropertyName = "sessionId")]
        public string SessionId { get; set; }

        [JsonProperty(PropertyName = "sessionCookie")]
        public string SessionCookie { get; set; }

        [JsonProperty(PropertyName = "initialPlayers")]
        public List<string> InitialPlayers { get; set; }

        public void CopyNonNullFields(SessionConfig other)
        {
            if (other == null)
            {
                return;
            }

            if (!string.IsNullOrEmpty(other.SessionId))
            {
                SessionId = other.SessionId;
            }

            if (!string.IsNullOrEmpty(other.SessionCookie))
            {
                SessionCookie = other.SessionCookie;
            }

            if (other.InitialPlayers != null && other.InitialPlayers.Any())
            {
                InitialPlayers = other.InitialPlayers;
            }
        }
    }
}