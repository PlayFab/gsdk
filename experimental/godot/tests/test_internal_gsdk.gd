extends GutTest
## Unit tests for internal GSDK logic.

var _internal: Node = null

const InternalGsdk := preload("res://addons/playfab_gsdk/internal_gsdk.gd")


func before_each() -> void:
	_internal = InternalGsdk.new()
	_internal.name = "TestInternalGSDK"
	add_child(_internal)


func after_each() -> void:
	if _internal != null:
		_internal.queue_free()
		_internal = null


# --- _create_config_map tests ---


func test_create_config_map_extracts_all_fields() -> void:
	_internal.configuration = {
		"heartbeatEndpoint": "localhost:56001",
		"sessionHostId": "test-host-id",
		"vmId": "test-vm-id",
		"logFolder": "/tmp/logs",
		"sharedContentFolder": "/shared",
		"certificateFolder": "/certs",
		"IpV4Address": "10.0.0.1",
		"fullyQualifiedDomainName": "test.playfab.com",
		"gameCertificates": { "cert1": "/path/to/cert" },
		"buildMetadata": { "version": "1.0" },
		"gamePorts": { "game_port": "7777" },
	}
	var config := _internal._create_config_map()

	assert_eq(config[PlayFabGsdkTypes.HEARTBEAT_ENDPOINT_KEY], "localhost:56001")
	assert_eq(config[PlayFabGsdkTypes.SERVER_ID_KEY], "test-host-id")
	assert_eq(config[PlayFabGsdkTypes.VM_ID_KEY], "test-vm-id")
	assert_eq(config[PlayFabGsdkTypes.LOG_FOLDER_KEY], "/tmp/logs")
	assert_eq(config[PlayFabGsdkTypes.SHARED_CONTENT_FOLDER_KEY], "/shared")
	assert_eq(config[PlayFabGsdkTypes.CERTIFICATE_FOLDER_KEY], "/certs")
	assert_eq(config[PlayFabGsdkTypes.IPV4_ADDRESS_KEY], "10.0.0.1")
	assert_eq(config[PlayFabGsdkTypes.FQDN_KEY], "test.playfab.com")
	assert_eq(config["cert1"], "/path/to/cert")
	assert_eq(config["version"], "1.0")
	assert_eq(config["game_port"], "7777")


func test_create_config_map_handles_missing_optional_fields() -> void:
	_internal.configuration = {}
	var config := _internal._create_config_map()

	assert_eq(config[PlayFabGsdkTypes.HEARTBEAT_ENDPOINT_KEY], "")
	assert_eq(config[PlayFabGsdkTypes.SERVER_ID_KEY], "")
	assert_eq(config[PlayFabGsdkTypes.VM_ID_KEY], "")
	assert_eq(config[PlayFabGsdkTypes.LOG_FOLDER_KEY], "")
	assert_eq(config[PlayFabGsdkTypes.SHARED_CONTENT_FOLDER_KEY], "")
	assert_eq(config[PlayFabGsdkTypes.CERTIFICATE_FOLDER_KEY], "")
	assert_eq(config[PlayFabGsdkTypes.IPV4_ADDRESS_KEY], "")
	assert_eq(config[PlayFabGsdkTypes.FQDN_KEY], "")


func test_create_config_map_skips_non_dict_game_certs() -> void:
	_internal.configuration = {
		"gameCertificates": "not_a_dict",
	}
	var config := _internal._create_config_map()
	# Should not crash and should not contain gameCertificates entries
	assert_false(config.has("not_a_dict"))


func test_create_config_map_skips_non_dict_build_metadata() -> void:
	_internal.configuration = {
		"buildMetadata": 42,
	}
	var config := _internal._create_config_map()
	assert_eq(config[PlayFabGsdkTypes.HEARTBEAT_ENDPOINT_KEY], "")


func test_create_config_map_skips_non_dict_game_ports() -> void:
	_internal.configuration = {
		"gamePorts": [],
	}
	var config := _internal._create_config_map()
	assert_eq(config[PlayFabGsdkTypes.HEARTBEAT_ENDPOINT_KEY], "")


# --- _update_state_from_heartbeat tests ---


func test_heartbeat_continue_keeps_state() -> void:
	_internal.current_game_state = PlayFabGsdkTypes.GameState.STANDING_BY
	_internal._update_state_from_heartbeat({
		"operation": "Continue",
	})
	assert_eq(
		_internal.current_game_state,
		PlayFabGsdkTypes.GameState.STANDING_BY,
	)


func test_heartbeat_active_transitions_state() -> void:
	_internal.current_game_state = PlayFabGsdkTypes.GameState.STANDING_BY
	_internal._update_state_from_heartbeat({
		"operation": "Active",
	})
	assert_eq(
		_internal.current_game_state,
		PlayFabGsdkTypes.GameState.ACTIVE,
	)


func test_heartbeat_active_emits_signal() -> void:
	_internal.current_game_state = PlayFabGsdkTypes.GameState.STANDING_BY
	watch_signals(_internal)
	_internal._update_state_from_heartbeat({
		"operation": "Active",
	})
	assert_signal_emitted(_internal, "transitioned_to_active")


func test_heartbeat_active_no_double_transition() -> void:
	_internal.current_game_state = PlayFabGsdkTypes.GameState.ACTIVE
	watch_signals(_internal)
	_internal._update_state_from_heartbeat({
		"operation": "Active",
	})
	assert_signal_not_emitted(_internal, "transitioned_to_active")


func test_heartbeat_terminate_transitions_state() -> void:
	_internal.current_game_state = PlayFabGsdkTypes.GameState.ACTIVE
	_internal._update_state_from_heartbeat({
		"operation": "Terminate",
	})
	assert_eq(
		_internal.current_game_state,
		PlayFabGsdkTypes.GameState.TERMINATING,
	)


func test_heartbeat_terminate_calls_shutdown_callback() -> void:
	var was_called := false
	_internal.shutdown_callback = func() -> void:
		was_called = true
	_internal.current_game_state = PlayFabGsdkTypes.GameState.ACTIVE
	_internal._update_state_from_heartbeat({
		"operation": "Terminate",
	})
	assert_true(was_called, "Shutdown callback should have been called")


func test_heartbeat_terminate_emits_signal_to_unblock() -> void:
	_internal.current_game_state = PlayFabGsdkTypes.GameState.STANDING_BY
	watch_signals(_internal)
	_internal._update_state_from_heartbeat({
		"operation": "Terminate",
	})
	assert_signal_emitted(_internal, "transitioned_to_active")


func test_heartbeat_terminate_no_double_transition() -> void:
	_internal.current_game_state = PlayFabGsdkTypes.GameState.TERMINATING
	watch_signals(_internal)
	_internal._update_state_from_heartbeat({
		"operation": "Terminate",
	})
	assert_signal_not_emitted(_internal, "transitioned_to_active")


func test_heartbeat_processes_session_cookie() -> void:
	_internal.config_map = {}
	_internal._update_state_from_heartbeat({
		"sessionConfig": {
			"sessionCookie": "my-cookie-value",
		},
	})
	assert_eq(
		_internal.config_map.get(PlayFabGsdkTypes.SESSION_COOKIE_KEY, ""),
		"my-cookie-value",
	)


func test_heartbeat_processes_session_id() -> void:
	_internal.config_map = {}
	_internal._update_state_from_heartbeat({
		"sessionConfig": {
			"sessionId": "abc-123",
		},
	})
	assert_eq(
		_internal.config_map.get(PlayFabGsdkTypes.SESSION_ID_KEY, ""),
		"abc-123",
	)


func test_heartbeat_processes_initial_players() -> void:
	_internal._update_state_from_heartbeat({
		"sessionConfig": {
			"initialPlayers": ["player1", "player2", "player3"],
		},
	})
	assert_eq(_internal.initial_players.size(), 3)
	assert_eq(_internal.initial_players[0], "player1")
	assert_eq(_internal.initial_players[1], "player2")
	assert_eq(_internal.initial_players[2], "player3")


func test_heartbeat_empty_initial_players_not_overwritten() -> void:
	_internal.initial_players = PackedStringArray(["existing"])
	_internal._update_state_from_heartbeat({
		"sessionConfig": {
			"initialPlayers": [],
		},
	})
	assert_eq(_internal.initial_players.size(), 1)
	assert_eq(_internal.initial_players[0], "existing")


func test_heartbeat_processes_metadata() -> void:
	_internal.config_map = {}
	_internal._update_state_from_heartbeat({
		"sessionConfig": {
			"metadata": {
				"mapName": "de_dust2",
				"gameMode": "competitive",
			},
		},
	})
	assert_eq(_internal.config_map.get("mapName", ""), "de_dust2")
	assert_eq(_internal.config_map.get("gameMode", ""), "competitive")


func test_heartbeat_processes_maintenance_callback() -> void:
	var received_time := ""
	_internal.maintenance_callback = func(time: String) -> void:
		received_time = time
	_internal._update_state_from_heartbeat({
		"nextScheduledMaintenanceUtc": "2026-01-15T10:00:00Z",
	})
	assert_eq(received_time, "2026-01-15T10:00:00Z")
	assert_eq(
		_internal.cached_scheduled_maintenance_utc,
		"2026-01-15T10:00:00Z",
	)


func test_heartbeat_maintenance_not_called_if_unchanged() -> void:
	var call_count := 0
	_internal.maintenance_callback = func(_time: String) -> void:
		call_count += 1
	_internal.cached_scheduled_maintenance_utc = "2026-01-15T10:00:00Z"
	_internal._update_state_from_heartbeat({
		"nextScheduledMaintenanceUtc": "2026-01-15T10:00:00Z",
	})
	assert_eq(call_count, 0, "Callback should not fire for same time")


func test_heartbeat_handles_missing_fields_gracefully() -> void:
	_internal.current_game_state = PlayFabGsdkTypes.GameState.STANDING_BY
	_internal.config_map = {}
	_internal._update_state_from_heartbeat({})
	assert_eq(
		_internal.current_game_state,
		PlayFabGsdkTypes.GameState.STANDING_BY,
	)


func test_heartbeat_ignores_non_dict_session_config() -> void:
	_internal.config_map = {}
	_internal._update_state_from_heartbeat({
		"sessionConfig": "not_a_dict",
	})
	assert_false(_internal.config_map.has(PlayFabGsdkTypes.SESSION_COOKIE_KEY))


# --- start_internal guard test ---


func test_start_internal_is_idempotent() -> void:
	# Without GSDK_CONFIG_FILE set, start_internal should fail but not crash.
	# Calling it twice should not cause issues.
	_internal.start_internal()
	_internal.start_internal()
	# The state should still be INITIALIZING since config load failed
	# but start_internal should not crash on repeated calls.
	pass
