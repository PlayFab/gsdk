namespace PlayFab.MultiplayerAgent.Model
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using Helpers;

    [Serializable]
    public class SessionConfig : IEquatable<SessionConfig>
    {
        [JsonProperty(PropertyName = "sessionId")]
        public string SessionId { get; set; }

        [JsonProperty(PropertyName = "sessionCookie")]
        public string SessionCookie { get; set; }

        [JsonProperty(PropertyName = "initialPlayers")]
        public List<string> InitialPlayers { get; set; }
        
        [JsonProperty(PropertyName = "metadata")]
        public Dictionary<string, string> Metadata { get; set; }
        
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
            
           if (other.Metadata != null && other.Metadata.Any())
           {
               Metadata = other.Metadata;
           }
            
        }
        
        public override bool Equals(object obj)
        {
            return Equals(obj as SessionConfig);
        }

        public bool Equals(SessionConfig other)
        {
            return other != null &&
                   SessionId == other.SessionId &&
                   SessionCookie == other.SessionCookie &&
                   EqualityComparer<List<string>>.Default.Equals(InitialPlayers, other.InitialPlayers) &&
                   EqualityComparer<Dictionary<string, string>>.Default.Equals(Metadata, other.Metadata);
        }

        public override int GetHashCode()
        {
            var hashCode = -481859842;
            hashCode = hashCode * -1521134295 + EqualityComparer<string>.Default.GetHashCode(SessionId);
            hashCode = hashCode * -1521134295 + EqualityComparer<string>.Default.GetHashCode(SessionCookie);
            hashCode = hashCode * -1521134295 + EqualityComparer<List<string>>.Default.GetHashCode(InitialPlayers);
            hashCode = hashCode * -1521134295 + EqualityComparer<Dictionary<string, string>>.Default.GetHashCode(Metadata);

            return hashCode;
        }

        public static bool operator ==(SessionConfig left, SessionConfig right)
        {
            return EqualityComparer<SessionConfig>.Default.Equals(left, right);
        }

        public static bool operator !=(SessionConfig left, SessionConfig right)
        {
            return !(left == right);
        }
    }
}
