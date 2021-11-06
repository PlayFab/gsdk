package gsdk

import "os"

// GameState represents the current state of the game.
type GameState string

// GameOperation represents the type of operation that the GSDK shoud do next
type GameOperation string

const (
	GameStateInvalid      GameState = "Invalid"
	GameStateInitializing GameState = "Initializing"
	GameStateStandingBy   GameState = "StandingBy"
	GameStateActive       GameState = "Active"
	GameStateTerminating  GameState = "Terminating"
	GameStateTerminated   GameState = "Terminated"
	GameStateQuarantined  GameState = "Quarantined" // Not used
)

const (
	SessionCookieKey            = "sessionCookie"
	SessionIdKey                = "sessionId"
	HeartbeatEndpointKey        = "heartbeatEndpoint"
	ServerIdKey                 = "serverId"
	LogFolderKey                = "logFolder"
	SharedContentFolderKey      = "sharedContentFolder"
	CertificateFolderKey        = "certificateFolder"
	TitleIdKey                  = "titleId"
	BuildIdKey                  = "buildId"
	RegionKey                   = "region"
	VmIdKey                     = "vmId"
	IpV4AddressKey              = "IpV4Address"
	FullyQualifiedDomainNameKey = "fullyQualifiedDomainName"
)

const (
	GameOperationInvalid   GameOperation = "Invalid"
	GameOperationContinue  GameOperation = "Continue"
	GameOperationActive    GameOperation = "Active"
	GameOperationTerminate GameOperation = "Terminate"
)

// HeartbeatRequest contains data for the heartbeat request coming from the GSDK running alongside GameServer
type HeartbeatRequest struct {
	// CurrentGameState is the current state of the game server
	CurrentGameState GameState `json:"CurrentGameState"`
	// CurrentGameHealth is the current health of the game server
	CurrentGameHealth string `json:"CurrentGameHealth"`
	// CurrentPlayers is a slice containing details about the players currently connected to the game
	CurrentPlayers []ConnectedPlayer `json:"CurrentPlayers"`
}

// HeartbeatResponse contains data for the heartbeat response that is being sent to the GSDK running alongside GameServer
type HeartbeatResponse struct {
	SessionConfig               *SessionConfig `json:"sessionConfig,omitempty"`
	NextScheduledMaintenanceUtc string         `json:"nextScheduledMaintenanceUtc,omitempty"`
	Operation                   GameOperation  `json:"operation,omitempty"`
}

// SessionConfig contains data for the session config that is being sent to the GSDK running alongside GameServer
type SessionConfig struct {
	SessionId      string            `json:"sessionId,omitempty"`
	SessionCookie  string            `json:"sessionCookie,omitempty"`
	InitialPlayers []string          `json:"initialPlayers,omitempty"`
	Metadata       map[string]string `json:"metadata,omitempty"`
}

// ConnectedPlayer contains data for a player connected to the game
type ConnectedPlayer struct {
	PlayerId string
}

// GsdkConfig is the configuration for the GSDK
// it will be written to the file that will be read by the GSDK running alongside the GameServer
type GsdkConfig struct {
	HeartbeatEndpoint        string                   `json:"heartbeatEndpoint"`
	SessionHostId            string                   `json:"sessionHostId"`
	VmId                     string                   `json:"vmId"`
	LogFolder                string                   `json:"logFolder"`
	CertificateFolder        string                   `json:"certificateFolder"`
	GameCertificates         map[string]string        `json:"gameCertificates"`
	SharedContentFolder      string                   `json:"sharedContentFolder"`
	BuildMetadata            map[string]string        `json:"buildMetadata"`
	GamePorts                map[string]int           `json:"gamePorts"`
	IpV4Address              string                   `json:"IpV4Address"`
	GameServerConnectionInfo GameServerConnectionInfo `json:"gameServerConnectionInfo"`
	ServerInstanceNumber     int                      `json:"serverInstanceNumber"`
	FullyQualifiedDomainName string                   `json:"fullyQualifiedDomainName"`
}

// GameServerConnectionInfo returns the connection info for the GameServer
type GameServerConnectionInfo struct {
	IpV4Address            string     `json:"IpV4Address"`
	GamePortsConfiguration []GamePort `json:"gamePortsConfiguration"`
}

// GamePort returns port connection information
type GamePort struct {
	Name                 string `json:"name"`
	ServerListeningPort  int    `json:"serverListeningPort"`
	ClientConnectionPort int    `json:"clientConnectionPort"`
}

// TitleId returns the PlayFab Title ID
func (g *GsdkConfig) TitleId() string {
	return os.Getenv("PF_TITLE_ID")
}

// BuildId returns the PlayFab Build ID
func (g *GsdkConfig) BuildId() string {
	return os.Getenv("PF_BUILD_ID")
}

// Region returns the region this Build is deployed
func (g *GsdkConfig) Region() string {
	return os.Getenv("PF_REGION")
}
