# Godot GSDK

This is an implementation of the PlayFab Game Server SDK (GSDK) for the [Godot Engine](https://godotengine.org/) (4.x) using GDScript. It's considered experimental and is not yet ready for production use or supported. Expect bugs and breaking changes :)

This version of GSDK is similar to the existing ones (C#, Java, C++, Go), apart from the fact that it contains a `mark_allocated()` method which internally sets the game server to active.

## Requirements

- Godot Engine 4.x

## Installation

1. Copy the `addons/playfab_gsdk/` directory into your Godot project's `addons/` folder.
2. In the Godot Editor, go to **Project > Project Settings > Plugins** and enable the **PlayFab GSDK** plugin. This will automatically register the `PlayFabGSDK` autoload singleton.

Alternatively, you can manually add the autoload:
1. Go to **Project > Project Settings > Globals > AutoLoad**.
2. Add `addons/playfab_gsdk/gsdk.gd` with the name `PlayFabGSDK`.

## Files

| File | Description |
|------|-------------|
| `gsdk.gd` | Public API — autoload singleton with methods like `start()`, `ready_for_players()`, `register_health_callback()`, etc. |
| `internal_gsdk.gd` | Internal implementation — handles heartbeat loop, state transitions, configuration management, and callbacks. |
| `types.gd` | Type definitions — game state and operation enums, configuration key constants, and serialization helpers. |
| `gsdk_logger.gd` | Logging — writes log messages to both the Godot console and a log file. |
| `playfab_gsdk_plugin.gd` | Editor plugin — registers the `PlayFabGSDK` autoload when the plugin is enabled. |
| `plugin.cfg` | Plugin descriptor for the Godot editor. |

## Things to remember

- `ready_for_players()` is async. It blocks using `await` until the game server transitions to Active. Use `await PlayFabGSDK.ready_for_players()`.
- `mark_allocated()` works only when the game server is in StandingBy state, so it should be called after `ready_for_players()` is called. For the time being, this method *should not* be used since it requires configuration settings on the Multiplayer Servers backend for each title.
- `RequestMultiplayerServer` API must *not* be called on a Build that uses `mark_allocated()`. It will probably work on the server side, but there is a small chance of concurrency issues if the two operations (`RequestMultiplayerServer` and `mark_allocated`) happen at the same time.

## Configuration

The GSDK reads its configuration from a JSON file whose path is specified by the `GSDK_CONFIG_FILE` environment variable. This file is automatically provided by the PlayFab VM agent when running on PlayFab Multiplayer Servers.

Additionally, the following environment variables are read:
- `PF_TITLE_ID` — PlayFab Title ID
- `PF_BUILD_ID` — PlayFab Build ID
- `PF_REGION` — Region where the build is deployed

## Usage

Here is a sample of calling the GSDK from your game server's main script:

```gdscript
extends Node

func _ready() -> void:
    # Register callbacks
    PlayFabGSDK.register_health_callback(_on_health_check)
    PlayFabGSDK.register_shutdown_callback(_on_shutdown)
    PlayFabGSDK.register_maintenance_callback(_on_maintenance)

    # Start the GSDK and wait for allocation
    _start_gsdk()

func _start_gsdk() -> void:
    PlayFabGSDK.log_message("Before ReadyForPlayers")
    var is_active := await PlayFabGSDK.ready_for_players()
    if is_active:
        PlayFabGSDK.log_message("Server is now active and ready for players!")
    else:
        PlayFabGSDK.log_message("Server failed to transition to active state")

func _on_health_check() -> bool:
    # Return true if the server is healthy, false otherwise
    return true

func _on_shutdown() -> void:
    PlayFabGSDK.log_message("Server is shutting down")
    get_tree().quit()

func _on_maintenance(maintenance_time: String) -> void:
    PlayFabGSDK.log_message("Maintenance scheduled at: %s" % maintenance_time)
```

### Using `mark_allocated()` (Experimental)

```gdscript
extends Node

func _ready() -> void:
    PlayFabGSDK.register_health_callback(func() -> bool: return true)
    PlayFabGSDK.register_shutdown_callback(func() -> void:
        PlayFabGSDK.log_message("Shutting down")
    )
    PlayFabGSDK.start()
    _do_mark_allocated()

func _do_mark_allocated() -> void:
    # Start a timer to mark allocated after some condition is met
    await get_tree().create_timer(300.0).timeout
    PlayFabGSDK.log_message("Marking allocated")
    var success := PlayFabGSDK.mark_allocated()
    if not success:
        PlayFabGSDK.log_message("Failed to mark allocated")
```

### Updating Connected Players

```gdscript
# Tell PlayFab about connected players
PlayFabGSDK.update_connected_players([
    {"PlayerId": "player-1"},
    {"PlayerId": "player-2"},
])
```

### Getting Configuration

```gdscript
# Get all configuration settings
var config := PlayFabGSDK.get_config_settings()
print("Server ID: ", config.get("serverId", ""))
print("Region: ", config.get("region", ""))

# Get specific directories
var logs_dir := PlayFabGSDK.get_logs_directory()
var shared_dir := PlayFabGSDK.get_shared_content_directory()

# Get connection info
var conn_info := PlayFabGSDK.get_game_server_connection_info()
```
