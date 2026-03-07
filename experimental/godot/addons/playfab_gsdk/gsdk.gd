## PlayFab Game Server SDK (GSDK) for Godot Engine.
##
## Add this script as an Autoload singleton named "PlayFabGSDK" in your Godot project,
## either manually or by enabling the PlayFab GSDK plugin.
## This provides the public API for integrating your Godot game server with
## PlayFab Multiplayer Servers.
extends Node

var _internal: Node = null


func _ready() -> void:
	var InternalGsdk := preload("res://addons/playfab_gsdk/internal_gsdk.gd")
	_internal = InternalGsdk.new()
	_internal.name = "InternalGSDK"
	add_child(_internal)


## Starts communication with the PlayFab Multiplayer Servers agent.
## Kicks off the heartbeat loop. This is called automatically by other methods
## if not called explicitly.
func start() -> void:
	_internal.start_internal()


## Starts communication with debug logging enabled.
func start_with_debug_logs() -> void:
	_internal.debug_logs = true
	_internal.start_internal()


## Transitions the game server state to StandingBy, telling the PlayFab service
## that the game server is ready to accept players.[br]
## [b]This is an async method[/b] — use: [code]await PlayFabGSDK.ready_for_players()[/code][br]
## Returns [code]true[/code] if the game server successfully transitioned to Active state.
func ready_for_players() -> bool:
	_internal.start_internal()
	if _internal.current_game_state != PlayFabGsdkTypes.GameState.ACTIVE:
		_internal.current_game_state = PlayFabGsdkTypes.GameState.STANDING_BY
		await _internal.transition_to_active
	return _internal.current_game_state == PlayFabGsdkTypes.GameState.ACTIVE


## [b]Experimental[/b] — Transitions the game server state to Active.[br]
## Only works when the game server is in StandingBy state.
## Returns [code]true[/code] on success, [code]false[/code] on failure.
func mark_allocated() -> bool:
	if _internal.current_game_state != PlayFabGsdkTypes.GameState.STANDING_BY:
		push_error("GSDK: Game server is not in the StandingBy state to be marked allocated")
		return false
	_internal.current_game_state = PlayFabGsdkTypes.GameState.ACTIVE
	_internal.transition_to_active.emit()
	return true


## Logs a message to the GSDK log output.
func log_message(message: String) -> void:
	_internal.start_internal()
	_internal._logger.log_info(message)


## Returns connection information (IP address and ports) for the game server.
func get_game_server_connection_info() -> Dictionary:
	_internal.start_internal()
	return _internal.configuration.get("gameServerConnectionInfo", {})


## Registers a health check callback. The callback should return [code]true[/code]
## for healthy, [code]false[/code] for unhealthy. It is called on each heartbeat.
func register_health_callback(callback: Callable) -> void:
	_internal.start_internal()
	_internal.health_callback = callback


## Registers a shutdown callback. Called when the server is being terminated
## by the PlayFab agent.
func register_shutdown_callback(callback: Callable) -> void:
	_internal.start_internal()
	_internal.shutdown_callback = callback


## Registers a maintenance callback. Called when a scheduled maintenance event
## is approaching. The callback receives the maintenance datetime as a [String]
## in ISO 8601 / RFC 3339 format.
func register_maintenance_callback(callback: Callable) -> void:
	_internal.start_internal()
	_internal.maintenance_callback = callback


## Returns the directory path for log files that will be uploaded to PlayFab.
func get_logs_directory() -> String:
	_internal.start_internal()
	return _internal.config_map.get(PlayFabGsdkTypes.LOG_FOLDER_KEY, "")


## Returns the shared content directory path shared among all game servers on the VM.
func get_shared_content_directory() -> String:
	_internal.start_internal()
	return _internal.config_map.get(PlayFabGsdkTypes.SHARED_CONTENT_FOLDER_KEY, "")


## Returns the list of initial players that have access to this game server.
## Only available after game server allocation.
func get_initial_players() -> PackedStringArray:
	_internal.start_internal()
	return _internal.initial_players


## Updates the list of connected players. Each element should be a [Dictionary]
## with a [code]"PlayerId"[/code] key.[br]
## Example: [code][{"PlayerId": "player1"}, {"PlayerId": "player2"}][/code]
func update_connected_players(players: Array) -> void:
	_internal.start_internal()
	_internal.connected_players = players


## Returns all configuration settings as a [Dictionary].
func get_config_settings() -> Dictionary:
	_internal.start_internal()
	return _internal.config_map
