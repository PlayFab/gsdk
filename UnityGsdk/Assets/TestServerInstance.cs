using UnityEngine;
using PlayFab;
using PlayFab.MultiplayerAgent.Model;

public class TestServerInstance : MonoBehaviour
{
    private void Awake()
    {
        Debug.LogWarning("TestServerInstance.Awake() called");
        PlayFabMultiplayerAgentAPI.OnShutDownCallback += OnShutdown;
        PlayFabMultiplayerAgentAPI.OnServerActiveCallback += OnServerActive;
        PlayFabMultiplayerAgentAPI.OnMaintenanceV2Callback += OnMaintenanceV2;

        PlayFabMultiplayerAgentAPI.Start();
    }

    private void Start()
    {
        Debug.LogWarning("TestServerInstance.Start() called");
        PlayFabMultiplayerAgentAPI.ReadyForPlayers();
    }

    private void OnShutdown()
    {
        Debug.LogWarning("TestServerInstance.OnShutdown() called - exiting");
        Application.Quit();
    }

    // Unused in this sdk right now
    private bool OnHealthCheck()
    {
        Debug.LogWarning("TestServerInstance.OnHealthCheck() called - returning healthy");
        return true;
    }

    private void OnServerActive()
    {
        Debug.LogWarning("TestServerInstance.OnServerActive() called");
    }

    private void OnMaintenanceV2(MaintenanceSchedule schedule)
    {
        Debug.LogWarning($"TestServerInstance.OnMaintenanceV2() called with {schedule.Events[0].EventType}, {schedule.Events[0].EventStatus}, {schedule.Events[0].EventSource}, " +
                                                                            $"{schedule.Events[0].Resources[0]}, {schedule.Events[0].NotBefore}");
    }
}
