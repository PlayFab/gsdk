namespace PlayFab.MultiplayerAgent.Model
{
    using System;
    using Helpers;

    [Serializable]
    public class ConnectedPlayer
    {
        public ConnectedPlayer()
        {

        }

        public ConnectedPlayer(string playerid)
        {
            PlayerId = playerid;
        }

        [JsonProperty(PropertyName = "playerId")]
        public string PlayerId { get; set; }
    }
}