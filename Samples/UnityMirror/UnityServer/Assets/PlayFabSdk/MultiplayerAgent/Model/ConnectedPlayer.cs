namespace PlayFab.MultiplayerAgent.Model
{
    using System;
    using Json;

    [Serializable]
    public class ConnectedPlayer
    {
        public ConnectedPlayer(string playerid)
        {
            PlayerId = playerid;
        }

        [JsonProperty(PropertyName = "playerId")]
        public string PlayerId { get; set; }
    }
}