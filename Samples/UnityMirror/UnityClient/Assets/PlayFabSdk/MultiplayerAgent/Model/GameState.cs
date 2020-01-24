namespace PlayFab.MultiplayerAgent.Model
{
    using System;

    [Serializable]
    public enum GameState
    {
        Invalid,
        Initializing,
        StandingBy,
        Active,
        Terminating,
        Terminated,
        Quarantined
    }
}