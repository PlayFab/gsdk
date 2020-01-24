namespace PlayFab.MultiplayerAgent.Model
{
    using System;
    using System.Collections.Generic;

    [Serializable]
    public class HeartbeatRequest
    {
        public GameState CurrentGameState { get; set; }

        public string CurrentGameHealth { get; set; }

        public IList<ConnectedPlayer> CurrentPlayers { get; set; }
    }
}