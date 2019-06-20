namespace PlayFab.MultiplayerAgent.Model
{
    using System;

    [Serializable]
    public enum ErrorStates
    {
        Ok = 0,
        Retry30S = 30,
        Retry5M = 300,
        Retry10M = 600,
        Retry15M = 900,
        Cancelled = -1
    }
}