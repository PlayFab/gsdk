#if ENABLE_PLAYFABSERVER_API

using System;

namespace PlayFab
{
    using MultiplayerAgent.Model;
    using UnityEngine;

    public class PlayFabMultiplayerAgentView : MonoBehaviour
    {
        /// <summary>
        /// Static reference to the current active instance of the PlayFabMultiplayerAgentView.
        /// </summary>
        public static PlayFabMultiplayerAgentView Current { get; private set; } = null;
        
        private float _timer;

        /// <summary>
        /// Awake constructor
        /// </summary>
        private void Awake()
        {
            Debug.Log($"{Time.fixedTime} {nameof(PlayFabMultiplayerAgentView)} awake");
            // Check if the static instance already contains a reference:
            if(Current != null)
            {
                // Destroy this instance since we only ever need one PlayFabMultiplayerAgentView
                Destroy(gameObject);
                return;
            }
            else
            {
                // Need to keep this game object alive through scene changes.
                DontDestroyOnLoad(this);
                Current = this;
            }
        }

        private void Start()
        {
            PlayFabMultiplayerAgentAPI.IsProcessing = true;
            StartCoroutine(PlayFabMultiplayerAgentAPI.SendInfo());
        }

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
                StartCoroutine(PlayFabMultiplayerAgentAPI.SendHeartBeatRequest());
            }
            else if (PlayFabMultiplayerAgentAPI.CurrentState.CurrentGameState == GameState.Terminating)
            {
                PlayFabMultiplayerAgentAPI.CurrentState.CurrentGameState = GameState.Terminated;
                PlayFabMultiplayerAgentAPI.IsProcessing = true;
                _timer = 0f;
                StartCoroutine(PlayFabMultiplayerAgentAPI.SendHeartBeatRequest());
            }
        }
        
        /// <summary>
        /// Called when gameobject is destroyed
        /// </summary>
        private void OnDestroy()
        {
            Debug.Log($"{Time.fixedTime} {nameof(PlayFabMultiplayerAgentView)} destroyed");
        }
    }
}

#endif