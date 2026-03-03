# Unreal GSDK Rules

## Required Setup
- PlayFabGSDK plugin enabled in `.uproject` Plugins array
- C++ project (not Blueprint-only) — the plugin requires C++ compilation
- Dedicated server target: a `*Server.Target.cs` file with `Type = TargetType.Server`
- Unreal Engine 5.3+

## API Pattern
```cpp
// Get module reference
FPlayFabGSDKModule* GSDKModule = &FModuleManager::LoadModuleChecked<FPlayFabGSDKModule>("PlayFabGSDK");

// Bind delegates BEFORE ReadyForPlayers()
GSDKModule->OnShutdown.BindUObject(this, &AMyActor::OnShutdown);
GSDKModule->OnServerActive.BindUObject(this, &AMyActor::OnServerActive);
GSDKModule->OnHealthCheck.BindUObject(this, &AMyActor::OnHealthCheck);

// NON-BLOCKING — just sets state to StandBy
GSDKModule->ReadyForPlayers();
```

## Language-Specific Rules

| ID | Rule | Severity |
|---|---|---|
| UE1 | PlayFabGSDK listed in `.uproject` Plugins with `"Enabled": true` | Error |
| UE2 | A `*Server.Target.cs` exists with `Type = TargetType.Server` | Error |
| UE3 | `OnShutdown` delegate is bound before `ReadyForPlayers()` | Error |
| UE4 | Project has C++ source (a `Source/` directory with `.Build.cs` files) | Error |
| UE5 | Health check callback returns quickly (under the heartbeat interval) | Warning |

## Common Mistakes
- Blueprint-only project — PlayFabGSDK requires C++ compilation; convert project first
- Missing dedicated server target — GSDK only runs on dedicated servers, not listen servers
- Assuming `ReadyForPlayers()` blocks — in Unreal it's non-blocking (unlike C++/C#/Java SDKs)
- Not checking `GetIsUsingGsdk()` — if false, the VM Agent is not present (local dev)
