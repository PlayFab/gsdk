extends GutTest
## Unit tests for [PlayFabGsdkTypes].


func test_game_state_to_string_invalid() -> void:
	assert_eq(
		PlayFabGsdkTypes.game_state_to_string(PlayFabGsdkTypes.GameState.INVALID),
		"Invalid",
	)


func test_game_state_to_string_initializing() -> void:
	assert_eq(
		PlayFabGsdkTypes.game_state_to_string(PlayFabGsdkTypes.GameState.INITIALIZING),
		"Initializing",
	)


func test_game_state_to_string_standing_by() -> void:
	assert_eq(
		PlayFabGsdkTypes.game_state_to_string(PlayFabGsdkTypes.GameState.STANDING_BY),
		"StandingBy",
	)


func test_game_state_to_string_active() -> void:
	assert_eq(
		PlayFabGsdkTypes.game_state_to_string(PlayFabGsdkTypes.GameState.ACTIVE),
		"Active",
	)


func test_game_state_to_string_terminating() -> void:
	assert_eq(
		PlayFabGsdkTypes.game_state_to_string(PlayFabGsdkTypes.GameState.TERMINATING),
		"Terminating",
	)


func test_game_state_to_string_terminated() -> void:
	assert_eq(
		PlayFabGsdkTypes.game_state_to_string(PlayFabGsdkTypes.GameState.TERMINATED),
		"Terminated",
	)


func test_game_state_to_string_quarantined() -> void:
	assert_eq(
		PlayFabGsdkTypes.game_state_to_string(PlayFabGsdkTypes.GameState.QUARANTINED),
		"Quarantined",
	)


func test_string_to_game_operation_continue() -> void:
	assert_eq(
		PlayFabGsdkTypes.string_to_game_operation("Continue"),
		PlayFabGsdkTypes.GameOperation.CONTINUE,
	)


func test_string_to_game_operation_active() -> void:
	assert_eq(
		PlayFabGsdkTypes.string_to_game_operation("Active"),
		PlayFabGsdkTypes.GameOperation.ACTIVE,
	)


func test_string_to_game_operation_terminate() -> void:
	assert_eq(
		PlayFabGsdkTypes.string_to_game_operation("Terminate"),
		PlayFabGsdkTypes.GameOperation.TERMINATE,
	)


func test_string_to_game_operation_unknown_returns_invalid() -> void:
	assert_eq(
		PlayFabGsdkTypes.string_to_game_operation("UnknownOp"),
		PlayFabGsdkTypes.GameOperation.INVALID,
	)


func test_string_to_game_operation_empty_returns_invalid() -> void:
	assert_eq(
		PlayFabGsdkTypes.string_to_game_operation(""),
		PlayFabGsdkTypes.GameOperation.INVALID,
	)


func test_config_key_constants() -> void:
	assert_eq(PlayFabGsdkTypes.SESSION_COOKIE_KEY, "sessionCookie")
	assert_eq(PlayFabGsdkTypes.SESSION_ID_KEY, "sessionId")
	assert_eq(PlayFabGsdkTypes.HEARTBEAT_ENDPOINT_KEY, "heartbeatEndpoint")
	assert_eq(PlayFabGsdkTypes.SERVER_ID_KEY, "serverId")
	assert_eq(PlayFabGsdkTypes.LOG_FOLDER_KEY, "logFolder")
	assert_eq(PlayFabGsdkTypes.SHARED_CONTENT_FOLDER_KEY, "sharedContentFolder")
	assert_eq(PlayFabGsdkTypes.CERTIFICATE_FOLDER_KEY, "certificateFolder")
	assert_eq(PlayFabGsdkTypes.TITLE_ID_KEY, "titleId")
	assert_eq(PlayFabGsdkTypes.BUILD_ID_KEY, "buildId")
	assert_eq(PlayFabGsdkTypes.REGION_KEY, "region")
	assert_eq(PlayFabGsdkTypes.VM_ID_KEY, "vmId")
	assert_eq(PlayFabGsdkTypes.IPV4_ADDRESS_KEY, "publicIpV4Address")
	assert_eq(PlayFabGsdkTypes.FQDN_KEY, "fullyQualifiedDomainName")
