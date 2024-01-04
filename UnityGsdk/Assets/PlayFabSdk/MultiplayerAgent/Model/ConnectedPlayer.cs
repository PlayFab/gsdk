namespace PlayFab.MultiplayerAgent.Model
{
    using System;
    using Helpers;

    [Serializable]
    public class ConnectedPlayer
    {
        [JsonProperty(PropertyName = "playerId")]
        public string PlayerId { get; set; }
    }
}