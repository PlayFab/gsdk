namespace PlayFab.MultiplayerAgent.Model
{
    using System;

    [Serializable]
    public enum GameOperation
    {
        Invalid,
        Continue,
        Active,
        Terminate
    }
}