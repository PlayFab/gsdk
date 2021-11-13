package gsdk

import (
	"context"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"log"
	"net/http"
	"os"
	"testing"
	"time"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/suite"
)

const (
	GsdkConfigFile = "GsdkConfig.json"
	testPort       = 56005
)

// Define the suite, and absorb the built-in basic suite
// functionality from testify - including a T() method which
// returns the current testing context
type GsdkTestSuite struct {
	suite.Suite
}

// Make sure that VariableThatShouldStartAtFive is set to five
// before each test
func (suite *GsdkTestSuite) SetupSuite() {
	gsdkConfig := &GsdkConfig{
		HeartbeatEndpoint:        fmt.Sprintf("localhost:%d", testPort),
		SessionHostId:            "test-host",
		VmId:                     "test-vm",
		LogFolder:                "/tmp",
		CertificateFolder:        "./certs",
		SharedContentFolder:      "./shared",
		BuildMetadata:            map[string]string{"k": "v"},
		GamePorts:                map[string]string{"game": "8080"},
		IpV4Address:              "127.0.0.1",
		GameCertificates:         map[string]string{"game": "cert"},
		FullyQualifiedDomainName: "test.domain.com",
		GameServerConnectionInfo: GameServerConnectionInfo{
			IpV4Address: "127.0.0.1",
			GamePortsConfiguration: []GamePort{
				{
					Name:                 "game",
					ServerListeningPort:  8080,
					ClientConnectionPort: 56100,
				},
			},
		},
	}
	file, _ := json.MarshalIndent(gsdkConfig, "", " ")
	err := ioutil.WriteFile(GsdkConfigFile, file, 0644)
	if err != nil {
		panic(err)
	}

	os.Setenv("GSDK_CONFIG_FILE", GsdkConfigFile)
}

var timesRun int = 0

type httpServerNormalGsdk struct{}

func (httpServerNormalGsdk) ServeHTTP(w http.ResponseWriter, req *http.Request) {
	hr := HeartbeatResponse{
		Operation: GameOperationContinue,
	}

	if timesRun >= 3 && timesRun < 6 {
		hr = HeartbeatResponse{
			Operation: GameOperationActive,
			SessionConfig: &SessionConfig{
				SessionId:     "8781f954-17ee-4b3a-8f77-ab82b780d6ff",
				SessionCookie: "mycookie",
			},
		}
	}
	if timesRun >= 6 {
		hr = HeartbeatResponse{
			Operation: GameOperationTerminate,
		}
	}

	timesRun++
	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(hr)
}

type httpServerForMarkAllocatedGsdk struct{}

func (httpServerForMarkAllocatedGsdk) ServeHTTP(w http.ResponseWriter, req *http.Request) {
	hr := HeartbeatResponse{
		Operation: GameOperationContinue,
	}

	if timesRun >= 6 {
		hr = HeartbeatResponse{
			Operation: GameOperationTerminate,
		}
	}
	timesRun++
	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(hr)
}

// TestExample tests the "normal" GSDK path
func (suite *GsdkTestSuite) TestExample() {
	ig = newInternalGsdk()
	timesRun = 0
	srv := &http.Server{Addr: fmt.Sprintf(":%d", testPort), Handler: httpServerNormalGsdk{}}
	LogMessage("starting web server")
	go func() {
		if err := srv.ListenAndServe(); err != http.ErrServerClosed {
			// Error starting or closing listener:
			log.Printf("HTTP server ListenAndServe: %v", err)
		}
	}()

	Start()
	LogMessage("Waiting for Initializing state")
	assert.Eventually(suite.T(), func() bool {
		return ig.currentGameState == GameStateInitializing
	}, time.Second*5, time.Millisecond*500)
	go ReadyForPlayers()
	LogMessage("ReadyForPlayers has been called, waiting for StandingBy")
	assert.Eventually(suite.T(), func() bool {
		return ig.currentGameState == GameStateStandingBy
	}, time.Second*5, time.Millisecond*1000)
	LogMessage("Waiting for active state")
	assert.Eventually(suite.T(), func() bool {
		return ig.currentGameState == GameStateActive
	}, time.Second*5, time.Millisecond*1000)
	LogMessage("Waiting for terminating state")
	assert.Eventually(suite.T(), func() bool {
		return ig.currentGameState == GameStateTerminating
	}, time.Second*5, time.Millisecond*1000)
	srv.Shutdown(context.TODO())
}

// TestMarkAllocated tests the GSDK path with the MarkAllocated method
func (suite *GsdkTestSuite) TestMarkAllocated() {
	ig = newInternalGsdk()
	timesRun = 0
	srv := &http.Server{Addr: fmt.Sprintf(":%d", testPort), Handler: httpServerForMarkAllocatedGsdk{}}
	LogMessage("starting web server")
	go func() {
		if err := srv.ListenAndServe(); err != http.ErrServerClosed {
			// Error starting or closing listener:
			log.Printf("HTTP server ListenAndServe: %v", err)
		}
	}()

	Start()
	LogMessage("Waiting for Initializing state")
	assert.Eventually(suite.T(), func() bool {
		return ig.currentGameState == GameStateInitializing
	}, time.Second*5, time.Millisecond*500)
	go ReadyForPlayers()
	LogMessage("ReadyForPlayers has been called, waiting for StandingBy")
	assert.Eventually(suite.T(), func() bool {
		return ig.currentGameState == GameStateStandingBy
	}, time.Second*5, time.Millisecond*1000)
	err := MarkAllocated()
	assert.NoError(suite.T(), err)
	LogMessage("Waiting for active state")
	assert.Eventually(suite.T(), func() bool {
		return ig.currentGameState == GameStateActive
	}, time.Second*5, time.Millisecond*1000)
	LogMessage("Waiting for terminating state")
	assert.Eventually(suite.T(), func() bool {
		return ig.currentGameState == GameStateTerminating
	}, time.Second*15, time.Millisecond*1000)
	srv.Shutdown(context.TODO())
}

func (suite *GsdkTestSuite) TearDownSuite() {
	err := os.Remove(GsdkConfigFile)
	if err != nil {
		panic(err)
	}
}

// In order for 'go test' to run this suite, we need to create
// a normal test function and pass our suite to suite.Run
func TestGsdkTestSuite(t *testing.T) {
	suite.Run(t, new(GsdkTestSuite))
}
