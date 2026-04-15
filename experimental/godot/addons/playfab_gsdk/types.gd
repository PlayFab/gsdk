class_name PlayFabGsdkTypes
## Type definitions for the PlayFab Game Server SDK.
##
## Contains game state and operation enums, configuration key constants,
## and utility methods for serialization.


## Game server lifecycle states.
enum GameState {
	INVALID,
	INITIALIZING,
	STANDING_BY,
	ACTIVE,
	TERMINATING,
	TERMINATED,
	QUARANTINED,
}


## Game operations received from the PlayFab agent via heartbeat responses.
enum GameOperation {
	INVALID,
	CONTINUE,
	ACTIVE,
	TERMINATE,
}


## Configuration key constants matching the PlayFab GSDK specification.
const SESSION_COOKIE_KEY := "sessionCookie"
const SESSION_ID_KEY := "sessionId"
const HEARTBEAT_ENDPOINT_KEY := "heartbeatEndpoint"
const SERVER_ID_KEY := "serverId"
const LOG_FOLDER_KEY := "logFolder"
const SHARED_CONTENT_FOLDER_KEY := "sharedContentFolder"
const CERTIFICATE_FOLDER_KEY := "certificateFolder"
const TITLE_ID_KEY := "titleId"
const BUILD_ID_KEY := "buildId"
const REGION_KEY := "region"
const VM_ID_KEY := "vmId"
const IPV4_ADDRESS_KEY := "publicIpV4Address"
const FQDN_KEY := "fullyQualifiedDomainName"


## Converts a [enum GameState] value to its string representation for JSON serialization.
static func game_state_to_string(state: GameState) -> String:
	match state:
		GameState.INVALID:
			return "Invalid"
		GameState.INITIALIZING:
			return "Initializing"
		GameState.STANDING_BY:
			return "StandingBy"
		GameState.ACTIVE:
			return "Active"
		GameState.TERMINATING:
			return "Terminating"
		GameState.TERMINATED:
			return "Terminated"
		GameState.QUARANTINED:
			return "Quarantined"
	return "Invalid"


## Converts a JSON operation string to a [enum GameOperation] value.
static func string_to_game_operation(s: String) -> GameOperation:
	match s:
		"Continue":
			return GameOperation.CONTINUE
		"Active":
			return GameOperation.ACTIVE
		"Terminate":
			return GameOperation.TERMINATE
	return GameOperation.INVALID
