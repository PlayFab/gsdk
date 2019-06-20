namespace PlayFab.MultiplayerAgent.Model
{
    using System;

    [Serializable]
    public class ConnectedPlayer
    {
        public ConnectedPlayer(string playerid)
        {
            PlayerId = playerid;
        }

        public string PlayerId { get; set; }
    }
}