# Unity GSDK Rules

## Required Setup
- Scripting define `ENABLE_PLAYFABSERVER_API` must be set (in Player Settings → Scripting Define Symbols, or via `csc.rsp`)
- `GSDK_CONFIG_FILE` environment variable must point to the JSON config file at runtime
- Allow downloads over HTTP in Player Settings (heartbeats use plain HTTP to localhost)

## API Pattern
```csharp
// Correct order — subscribe events BEFORE Start()
PlayFabMultiplayerAgentAPI.OnShutDownCallback += OnShutdown;
PlayFabMultiplayerAgentAPI.OnServerActiveCallback += OnServerActive;
PlayFabMultiplayerAgentAPI.OnAgentErrorCallback += OnAgentError;

PlayFabMultiplayerAgentAPI.Start();
PlayFabMultiplayerAgentAPI.ReadyForPlayers(); // NON-BLOCKING (just sets state)

// Heartbeats must be sent via coroutine
StartCoroutine(PlayFabMultiplayerAgentAPI.SendHeartBeatRequest());
```

## Language-Specific Rules

| ID | Rule | Severity |
|---|---|---|
| UN1 | `ENABLE_PLAYFABSERVER_API` is in scripting defines | Error |
| UN2 | `PlayFabMultiplayerAgentAPI.Start()` is called | Error |
| UN3 | `OnShutDownCallback` is subscribed before `Start()` | Error |
| UN4 | Heartbeats use `StartCoroutine(SendHeartBeatRequest())`, not a blocking call in `Update()` | Warning |
| UN5 | `OnAgentErrorCallback` is handled (agent retries with exponential backoff up to 30 min, then cancels) | Info |
| UN6 | `ReadyForPlayers()` is not called multiple times (throws if already Active) | Warning |

## Common Mistakes
- Missing `ENABLE_PLAYFABSERVER_API` define — all GSDK code compiles out, server does nothing
- Not setting `GSDK_CONFIG_FILE` env var — `Start()` calls `Application.Quit()` immediately
- Calling `ReadyForPlayers()` twice — throws an exception on the second call
- Subscribing to events after `Start()` — may miss the initial state transition
- Blocking `Update()` with heartbeat logic — use coroutines instead
