package gsdk

import (
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
	"log"
	"net/http"
	"os"
	"strconv"
	"sync"
	"time"
)

type internalGsdk struct {
	currentGameState GameState
	// transitionToActiveEvent is used to block the ReadyForPlayers() method
	transitionToActiveEvent       chan interface{}
	transitionToActiveEventClosed bool
	// signalHeartbeatEvent is used to signal the heartbeat loop to send a heartbeat because of a state change
	signalHeartbeatEvent          chan interface{}
	configuration                 *GsdkConfig
	configMap                     map[string]string
	healthCallback                func() bool
	maintenanceCallback           func(time.Time)
	shutdownCallback              func()
	connectedPlayers              []ConnectedPlayer
	initialPlayers                []string
	cachedScheduledMaintenanceUtc time.Time
	startInternalRan              bool
	// startInternalMutex is used to ensure that the startCore() method is only called once
	startInternalMutex *sync.Mutex
	debugLogs          bool
}

// newInternalGsdk creates a new instance of the internalGsdk struct
func newInternalGsdk() *internalGsdk {
	return &internalGsdk{
		currentGameState:              "",
		transitionToActiveEvent:       make(chan interface{}),
		transitionToActiveEventClosed: false,
		signalHeartbeatEvent:          make(chan interface{}),
		configuration:                 nil,
		configMap:                     nil,
		healthCallback:                nil,
		maintenanceCallback:           nil,
		shutdownCallback:              nil,
		connectedPlayers:              nil,
		initialPlayers:                nil,
		startInternalRan:              false,
		startInternalMutex:            &sync.Mutex{},
	}
}

// startInternal initializes the core GSDK functionality
func (i *internalGsdk) startInternal() {
	i.startInternalMutex.Lock()
	defer i.startInternalMutex.Unlock()
	if i.startInternalRan {
		return
	}

	i.currentGameState = GameStateInitializing
	var err error
	if i.configuration == nil {
		i.configuration, err = i.getConfiguration()
		if err != nil {
			log.Fatal(err)
		}
	}

	if i.configMap == nil {
		i.configMap = i.createConfigMap(i.configuration)
	}

	heartbeatEndpoint := i.configMap[HeartbeatEndpointKey]
	serverId := i.configMap[ServerIdKey]

	initializeLogger(i.configMap[LogFolderKey])

	logInfo(fmt.Sprintf("VM Agent Endpoint %s", heartbeatEndpoint))
	logInfo(fmt.Sprintf("Instance Id %s", serverId))

	// run heartbeats on a different goroutine
	go i.heartbeat(fmt.Sprintf("http://%s/v1/sessionHosts/%s/heartbeats", heartbeatEndpoint, serverId))
	i.startInternalRan = true
}

func (i *internalGsdk) heartbeat(heartbeatEndpoint string) {
	// if we get a signal for a state transition, we'll heartbeat
	// else, we just heartbeat every second
	for {
		select {
		case <-i.signalHeartbeatEvent:
			if i.debugLogs {
				logInfo("State transition signaled an early heartbeat")
			}
			i.sendHeartbeat(heartbeatEndpoint)
		case <-time.After(time.Second * 1):
			i.sendHeartbeat(heartbeatEndpoint)
		}
	}
}

func (i *internalGsdk) sendHeartbeat(heartbeatEndpoint string) {
	gameHealth := false
	if i.healthCallback != nil {
		gameHealth = i.healthCallback()
	}
	currentGameHealth := "Healthy"
	if !gameHealth {
		currentGameHealth = "Unhealthy"
	}
	hrr := HeartbeatRequest{
		CurrentGameState:  i.currentGameState,
		CurrentGameHealth: currentGameHealth,
		CurrentPlayers:    i.connectedPlayers,
	}

	postBody, _ := json.Marshal(hrr)
	requestBody := bytes.NewBuffer(postBody)

	resp, err := http.Post(heartbeatEndpoint, "application/json", requestBody)
	if err != nil {
		logWarn(fmt.Sprintf("Error sending heartbeat %s", err))
		return
	}

	defer resp.Body.Close()
	body, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		logWarn(fmt.Sprintf("Error reading heartbeat response %s", err))
	}
	hbr := HeartbeatResponse{}
	if err := json.Unmarshal(body, &hbr); err != nil {
		logWarn(fmt.Sprintf("Error parsing heartbeat response %s", err))
	}

	i.updateStateFromHeartbeat(&hbr)

	if i.debugLogs {
		logDebug(fmt.Sprintf("Heartbeat request %#v, response %#v", hrr, hbr))
	}
}

func (i *internalGsdk) updateStateFromHeartbeat(hr *HeartbeatResponse) {
	if hr.SessionConfig != nil {
		i.configMap[SessionCookieKey] = hr.SessionConfig.SessionCookie
		i.configMap[SessionIdKey] = hr.SessionConfig.SessionId

		if hr.SessionConfig.InitialPlayers != nil && len(hr.SessionConfig.InitialPlayers) > 0 {
			i.initialPlayers = hr.SessionConfig.InitialPlayers
		}

		if hr.SessionConfig.Metadata != nil && len(hr.SessionConfig.Metadata) > 0 {
			for k, v := range hr.SessionConfig.Metadata {
				i.configMap[k] = v
			}
		}
	}

	if hr.NextScheduledMaintenanceUtc != "" {
		nextScheduledMaintenanceUtc, _ := time.Parse(time.RFC3339, hr.NextScheduledMaintenanceUtc)
		if i.cachedScheduledMaintenanceUtc.IsZero() || nextScheduledMaintenanceUtc.After(i.cachedScheduledMaintenanceUtc) {
			i.cachedScheduledMaintenanceUtc = nextScheduledMaintenanceUtc
			if i.maintenanceCallback != nil {
				i.maintenanceCallback(nextScheduledMaintenanceUtc)
			}
		}
	}

	switch hr.Operation {
	case GameOperationContinue:
		{
			// nothing to do
		}
	case GameOperationActive:
		{
			if i.currentGameState != GameStateActive {
				// game has transitioned to active
				i.currentGameState = GameStateActive
				// so, unblock ReadyForPlayers()
				i.transitionToActiveEvent <- struct{}{}
				// signal an early heartbeat
				go func() {
					i.signalHeartbeatEvent <- struct{}{}
				}()
			}
		}
	case GameOperationTerminate:
		{
			if i.currentGameState != GameStateTerminating {
				i.currentGameState = GameStateTerminating
				go func() {
					i.signalHeartbeatEvent <- struct{}{}
				}()
				if i.shutdownCallback != nil {
					i.shutdownCallback()
				}
				if i.transitionToActiveEventClosed {
					logWarn("trying to close already closed transitionToActiveEvent channel")
					return
				}
				// we're terminating, so signal that no more messages will be sent to the channe
				close(i.transitionToActiveEvent)
				i.transitionToActiveEventClosed = true
			}
		}
	}

}

func (i *internalGsdk) getConfiguration() (*GsdkConfig, error) {
	filename := os.Getenv("GSDK_CONFIG_FILE")
	if filename == "" {
		return nil, errors.New("GSDK_CONFIG_FILE environment variable not set")
	}
	jsonFile, err := os.Open(filename)
	if err != nil {
		return nil, err
	}
	defer jsonFile.Close()
	byteValue, err := ioutil.ReadAll(jsonFile)
	if err != nil {
		return nil, err
	}
	var gsdkConfig *GsdkConfig
	json.Unmarshal(byteValue, &gsdkConfig)
	return gsdkConfig, nil
}

func (i *internalGsdk) createConfigMap(gsdkConfig *GsdkConfig) map[string]string {
	configMap := make(map[string]string)
	for k, v := range gsdkConfig.GameCertificates {
		configMap[k] = v
	}
	for k, v := range gsdkConfig.BuildMetadata {
		configMap[k] = v
	}
	for k, v := range gsdkConfig.GamePorts {
		configMap[k] = strconv.Itoa(v)
	}

	configMap[HeartbeatEndpointKey] = gsdkConfig.HeartbeatEndpoint
	configMap[ServerIdKey] = gsdkConfig.SessionHostId
	configMap[VmIdKey] = gsdkConfig.VmId
	configMap[LogFolderKey] = gsdkConfig.LogFolder
	configMap[SharedContentFolderKey] = gsdkConfig.SharedContentFolder
	configMap[CertificateFolderKey] = gsdkConfig.CertificateFolder
	configMap[TitleIdKey] = gsdkConfig.TitleId()
	configMap[BuildIdKey] = gsdkConfig.BuildId()
	configMap[RegionKey] = gsdkConfig.Region()
	configMap[IpV4AddressKey] = gsdkConfig.IpV4Address
	configMap[FullyQualifiedDomainNameKey] = gsdkConfig.FullyQualifiedDomainName

	return configMap
}
