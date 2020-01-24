namespace PlayFab
{
    using MultiplayerAgent.Model;
    using UnityEngine;

    public class PlayFabMultiplayerAgentView : MonoBehaviour
    {
        private float _timer;

        private void LateUpdate()
        {
            if (PlayFabMultiplayerAgentAPI.CurrentState == null)
            {
                return;
            }

            float max = 1f;
            _timer += Time.deltaTime;
            if (PlayFabMultiplayerAgentAPI.CurrentErrorState != ErrorStates.Ok)
            {
                switch (PlayFabMultiplayerAgentAPI.CurrentErrorState)
                {
                    case ErrorStates.Retry30S:
                    case ErrorStates.Retry5M:
                    case ErrorStates.Retry10M:
                    case ErrorStates.Retry15M:
                        max = (float)PlayFabMultiplayerAgentAPI.CurrentErrorState;
                        break;
                    case ErrorStates.Cancelled:
                        max = 1f;
                        break;
                }
            }

            bool isTerminating = PlayFabMultiplayerAgentAPI.CurrentState.CurrentGameState == GameState.Terminated ||
                                 PlayFabMultiplayerAgentAPI.CurrentState.CurrentGameState == GameState.Terminating;
            bool isCancelled = PlayFabMultiplayerAgentAPI.CurrentErrorState == ErrorStates.Cancelled;

            if (!isTerminating && !isCancelled && !PlayFabMultiplayerAgentAPI.IsProcessing && _timer >= max)
            {
                if (PlayFabMultiplayerAgentAPI.IsDebugging)
                {
                    Debug.LogFormat("Timer:{0} - Max:{1}", _timer, max);
                }

                PlayFabMultiplayerAgentAPI.IsProcessing = true;
                _timer = 0f;
                PlayFabMultiplayerAgentAPI.SendHeartBeatRequest();
            }
            else if (PlayFabMultiplayerAgentAPI.CurrentState.CurrentGameState == GameState.Terminating)
            {
                PlayFabMultiplayerAgentAPI.CurrentState.CurrentGameState = GameState.Terminated;
                PlayFabMultiplayerAgentAPI.IsProcessing = true;
                _timer = 0f;
                PlayFabMultiplayerAgentAPI.SendHeartBeatRequest();
            }
        }
    }
}