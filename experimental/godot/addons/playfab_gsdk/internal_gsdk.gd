## Internal GSDK implementation.
##
## Handles heartbeat communication with the PlayFab Multiplayer Servers agent,
## game server state management, and configuration loading. This script is used
## internally by [code]gsdk.gd[/code] and should not be used directly.
extends Node

## Emitted when the game server transitions to the Active state.
signal transition_to_active

var current_game_state: PlayFabGsdkTypes.GameState = PlayFabGsdkTypes.GameState.INVALID
var configuration: Dictionary = {}
var config_map: Dictionary = {}
var health_callback: Callable = Callable()
var maintenance_callback: Callable = Callable()
var shutdown_callback: Callable = Callable()
var connected_players: Array = []
var initial_players: PackedStringArray = []
var cached_scheduled_maintenance_utc: String = ""
var debug_logs: bool = false

var _started: bool = false
var _logger = preload("gsdk_logger.gd").new()
var _heartbeat_timer: Timer = null
var _http_request: HTTPRequest = null
var _heartbeat_endpoint: String = ""
var _pending_heartbeat: bool = false


## Initializes the GSDK: loads configuration, sets up the heartbeat timer and
## HTTP client, and begins the heartbeat loop. Safe to call multiple times;
## initialization only runs once.
func start_internal() -> void:
	if _started:
		return

	current_game_state = PlayFabGsdkTypes.GameState.INITIALIZING

	var err := _load_configuration()
	if err != "":
		push_error("GSDK: Failed to load configuration: %s" % err)
		return

	config_map = _create_config_map()

	var log_folder := str(config_map.get(PlayFabGsdkTypes.LOG_FOLDER_KEY, ""))
	if log_folder != "":
		_logger.initialize(log_folder)

	var heartbeat_endpoint := str(config_map.get(PlayFabGsdkTypes.HEARTBEAT_ENDPOINT_KEY, ""))
	var server_id := str(config_map.get(PlayFabGsdkTypes.SERVER_ID_KEY, ""))

	_logger.log_info("VM Agent Endpoint %s" % heartbeat_endpoint)
	_logger.log_info("Instance Id %s" % server_id)

	_heartbeat_endpoint = "http://%s/v1/sessionHosts/%s/heartbeats" % [heartbeat_endpoint, server_id]

	# Set up heartbeat timer (fires every 1 second)
	_heartbeat_timer = Timer.new()
	_heartbeat_timer.wait_time = 1.0
	_heartbeat_timer.autostart = true
	_heartbeat_timer.timeout.connect(_on_heartbeat_timer)
	add_child(_heartbeat_timer)

	# Set up HTTP request node for heartbeat communication
	_http_request = HTTPRequest.new()
	_http_request.request_completed.connect(_on_heartbeat_response)
	add_child(_http_request)

	_started = true


func _on_heartbeat_timer() -> void:
	_send_heartbeat()


func _send_heartbeat() -> void:
	if _pending_heartbeat:
		return

	var game_health := false
	if health_callback.is_valid():
		game_health = health_callback.call()

	var current_game_health := "Healthy" if game_health else "Unhealthy"

	var players_array: Array = []
	for player in connected_players:
		if player is Dictionary:
			players_array.append({"PlayerId": player.get("PlayerId", "")})

	var heartbeat_request := {
		"CurrentGameState": PlayFabGsdkTypes.game_state_to_string(current_game_state),
		"CurrentGameHealth": current_game_health,
		"CurrentPlayers": players_array,
	}

	var json_body := JSON.stringify(heartbeat_request)
	var headers := PackedStringArray(["Content-Type: application/json"])

	_pending_heartbeat = true
	var error := _http_request.request(
		_heartbeat_endpoint,
		headers,
		HTTPClient.METHOD_POST,
		json_body
	)

	if error != OK:
		_logger.log_warn("Error sending heartbeat: %s" % str(error))
		_pending_heartbeat = false


func _on_heartbeat_response(result: int, response_code: int, _headers: PackedStringArray, body: PackedByteArray) -> void:
	_pending_heartbeat = false

	if result != HTTPRequest.RESULT_SUCCESS:
		_logger.log_warn("Error receiving heartbeat response: %d" % result)
		return

	if response_code < 200 or response_code >= 300:
		_logger.log_error("Error in heartbeat response: %d" % response_code)
		return

	var json_string := body.get_string_from_utf8()
	var json := JSON.new()
	var parse_result := json.parse(json_string)
	if parse_result != OK:
		_logger.log_warn("Error parsing heartbeat response: %s" % json.get_error_message())
		return

	var response: Dictionary = json.data
	_update_state_from_heartbeat(response)

	if debug_logs:
		_logger.log_debug("Heartbeat response: %s" % json_string)


func _update_state_from_heartbeat(response: Dictionary) -> void:
	# Process session config
	if response.has("sessionConfig") and response["sessionConfig"] is Dictionary:
		var session_config: Dictionary = response["sessionConfig"]

		if session_config.has("sessionCookie"):
			config_map[PlayFabGsdkTypes.SESSION_COOKIE_KEY] = str(session_config["sessionCookie"])
		if session_config.has("sessionId"):
			config_map[PlayFabGsdkTypes.SESSION_ID_KEY] = str(session_config["sessionId"])

		if session_config.has("initialPlayers") and session_config["initialPlayers"] is Array:
			var players: Array = session_config["initialPlayers"]
			if players.size() > 0:
				var string_players: PackedStringArray = []
				for p in players:
					string_players.append(str(p))
				initial_players = string_players

		if session_config.has("metadata") and session_config["metadata"] is Dictionary:
			var metadata: Dictionary = session_config["metadata"]
			for key in metadata:
				config_map[key] = str(metadata[key])

	# Process scheduled maintenance
	if response.has("nextScheduledMaintenanceUtc"):
		var maintenance_time := str(response["nextScheduledMaintenanceUtc"])
		if maintenance_time != "" and maintenance_time != cached_scheduled_maintenance_utc:
			cached_scheduled_maintenance_utc = maintenance_time
			if maintenance_callback.is_valid():
				maintenance_callback.call(maintenance_time)

	# Process operation from agent
	if response.has("operation"):
		var operation := PlayFabGsdkTypes.string_to_game_operation(str(response["operation"]))
		match operation:
			PlayFabGsdkTypes.GameOperation.CONTINUE:
				pass
			PlayFabGsdkTypes.GameOperation.ACTIVE:
				if current_game_state != PlayFabGsdkTypes.GameState.ACTIVE:
					current_game_state = PlayFabGsdkTypes.GameState.ACTIVE
					transition_to_active.emit()
					_send_heartbeat()
			PlayFabGsdkTypes.GameOperation.TERMINATE:
				if current_game_state != PlayFabGsdkTypes.GameState.TERMINATING:
					current_game_state = PlayFabGsdkTypes.GameState.TERMINATING
					_send_heartbeat()
					if shutdown_callback.is_valid():
						shutdown_callback.call()
					# Unblock ready_for_players() if it is waiting
					transition_to_active.emit()


func _load_configuration() -> String:
	var config_file := OS.get_environment("GSDK_CONFIG_FILE")
	if config_file == "":
		return "GSDK_CONFIG_FILE environment variable not set"

	if not FileAccess.file_exists(config_file):
		return "Configuration file not found: %s" % config_file

	var file := FileAccess.open(config_file, FileAccess.READ)
	if file == null:
		return "Failed to open configuration file: %s" % config_file

	var json_string := file.get_as_text()
	file.close()

	var json := JSON.new()
	var parse_result := json.parse(json_string)
	if parse_result != OK:
		return "Failed to parse configuration file: %s" % json.get_error_message()

	if not json.data is Dictionary:
		return "Configuration file does not contain a valid JSON object"

	configuration = json.data
	return ""


func _create_config_map() -> Dictionary:
	var cmap: Dictionary = {}

	if configuration.has("gameCertificates") and configuration["gameCertificates"] is Dictionary:
		for key in configuration["gameCertificates"]:
			cmap[key] = str(configuration["gameCertificates"][key])

	if configuration.has("buildMetadata") and configuration["buildMetadata"] is Dictionary:
		for key in configuration["buildMetadata"]:
			cmap[key] = str(configuration["buildMetadata"][key])

	if configuration.has("gamePorts") and configuration["gamePorts"] is Dictionary:
		for key in configuration["gamePorts"]:
			cmap[key] = str(configuration["gamePorts"][key])

	cmap[PlayFabGsdkTypes.HEARTBEAT_ENDPOINT_KEY] = str(configuration.get("heartbeatEndpoint", ""))
	cmap[PlayFabGsdkTypes.SERVER_ID_KEY] = str(configuration.get("sessionHostId", ""))
	cmap[PlayFabGsdkTypes.VM_ID_KEY] = str(configuration.get("vmId", ""))
	cmap[PlayFabGsdkTypes.LOG_FOLDER_KEY] = str(configuration.get("logFolder", ""))
	cmap[PlayFabGsdkTypes.SHARED_CONTENT_FOLDER_KEY] = str(configuration.get("sharedContentFolder", ""))
	cmap[PlayFabGsdkTypes.CERTIFICATE_FOLDER_KEY] = str(configuration.get("certificateFolder", ""))
	cmap[PlayFabGsdkTypes.TITLE_ID_KEY] = OS.get_environment("PF_TITLE_ID")
	cmap[PlayFabGsdkTypes.BUILD_ID_KEY] = OS.get_environment("PF_BUILD_ID")
	cmap[PlayFabGsdkTypes.REGION_KEY] = OS.get_environment("PF_REGION")
	cmap[PlayFabGsdkTypes.IPV4_ADDRESS_KEY] = str(configuration.get("IpV4Address", ""))
	cmap[PlayFabGsdkTypes.FQDN_KEY] = str(configuration.get("fullyQualifiedDomainName", ""))

	return cmap
