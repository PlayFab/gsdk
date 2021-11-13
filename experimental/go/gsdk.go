package gsdk

import (
	"errors"
	"time"
)

var (
	ig = newInternalGsdk()
)

// Start kicks off communication threads, heartbeats, etc.
func Start() {
	ig.startInternal()
}

// StartWithDebugLogs is like Start(), but with debug logs enabled
func StartWithDebugLogs() {
	ig.debugLogs = true
	ig.startInternal()
}

// ReadyForPlayers transitions the state to standingBy which tells the PlayFab service that the game server is ready to accept players
// Be aware that it is a *blocking* call, it unblocks when the game server is allocated (transitions to Active)
func ReadyForPlayers() bool {
	ig.startInternal()
	if ig.currentGameState != GameStateActive {
		ig.currentGameState = GameStateStandingBy
		<-ig.transitionToActiveEvent
	}
	return ig.currentGameState == GameStateActive
}

// MarkAllocated is an experimental method that transitions the state to Active
func MarkAllocated() error {
	if ig.currentGameState != GameStateStandingBy {
		return errors.New("game server is not in the standing by state to be marked allocated")
	}
	ig.currentGameState = GameStateActive
	ig.transitionToActiveEvent <- struct{}{}
	return nil
}

// LogMessage adds a custom log message to the GSDK log output
func LogMessage(s string) {
	ig.startInternal()
	logInfo(s)
}

// GetGameServerConnectionInfo gets information (ipAddress and ports) for connecting to the game server, as well as the ports the game server should listen on.
func GetGameServerConnectionInfo() GameServerConnectionInfo {
	ig.startInternal()
	return ig.configuration.GameServerConnectionInfo
}

// RegisterMaintenanceCallback gets called if the server is getting a scheduled maintenance, it will get the UTC Datetime of the maintenance event as an argument.
func RegisterMaintenanceCallback(callback func(time.Time)) {
	ig.startInternal()
	ig.maintenanceCallback = callback
}

// RegisterHealthCallback gets called when the agent needs to check on the game's health
func RegisterHealthCallback(callback func() bool) {
	ig.startInternal()
	ig.healthCallback = callback
}

// RegisterShutdownCallback gets called if the server is shutting us down
func RegisterShutdownCallback(callback func()) {
	ig.startInternal()
	ig.shutdownCallback = callback
}

// GetLogsDirectory returns the directory whose contents will be uploaded so logs can be easily retrieved
func GetLogsDirectory() string {
	ig.startInternal()
	return ig.configMap[LogFolderKey]
}

// GetSharedContentDirectory returns the directory whose contents are shared among all game servers within a VM
func GetSharedContentDirectory() string {
	ig.startInternal()
	return ig.configMap[SharedContentFolderKey]
}

// GetInitialPlayers returns a list of the initial players that have access to this game server. It works only after game server allocation
func GetInitialPlayers() []string {
	ig.startInternal()
	return ig.initialPlayers
}

// UpdateConnectedPlayers tells the PlayFab service information on who is connected
func UpdateConnectedPlayers(players []ConnectedPlayer) {
	ig.startInternal()
	ig.connectedPlayers = players
}

// GetConfigSettings returns the configuration settings for this game server
func GetConfigSettings() map[string]string {
	ig.startInternal()
	return ig.configMap
}
