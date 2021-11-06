# Go GSDK

This is an implementation of the GSDK in Go programming language. It's considered experimental and is not yet ready for production use or supported. Expect bugs and breaking changes :)

This version of GSDK is pretty similar to the existing ones, apart from the fact that it contains a "MarkAllocated" method which internally sets the game server to active.

Things to remember:

- ReadyForPlayers is blocking, it unblocks when the game server is Active. However, it can always be used in a goroutine
- MarkAllocated woks only when the game server is on StandingBy, to it should be called after "ReadyForPlayers" is called
- RequestMultiplayerServer API must *not* be called on a Build that uses "MarkAllocated". It will probably work on the server side, but there is a small chance of concurrency issues if the two operations (RequestMultiplayerServer and MarkAllocated) happen at the same time.

Here is a sample of calling GSDK:

```golang
package main

import (
	"fmt"
	"time"

	gsdk "github.com/playfab/gsdk/experimental/go"
)

func main() {
	gsdk.RegisterHealthCallback(func() bool {
		return true
	})
	gsdk.RegisterShutdownCallback(func() {
		gsdk.LogMessage("Shutting down")
	})
	gsdk.Start()
	doMarkAllocatedGSDK()
}

// doMarkAllocatedGSDK will start a goroutine which will call MarkAllocated after 5 minutes
func doMarkAllocatedGSDK() {
	go func() {
		gsdk.LogMessage("Sleeping for 300 seconds")
		time.Sleep(time.Second * 300)
		gsdk.LogMessage("Marking allocated")
		if err := gsdk.MarkAllocated(); err != nil {
			gsdk.LogMessage(fmt.Sprintf("Error marking allocated: %v", err))
		}
	}()
	gsdk.LogMessage("Before ReadyForPlayers")
	gsdk.ReadyForPlayers()
	gsdk.LogMessage("After ReadyForPlayers")
	time.Sleep(time.Second * 1200)
}

// doNormalGSDK is the "normal" way to call GSDK, with ReadyForPlayers blocking till the RequestMultiplayerServer API is called
func doNormalGSDK() {
	gsdk.RegisterHealthCallback(func() bool {
		return true
	})
	gsdk.RegisterShutdownCallback(func() {
		gsdk.LogMessage("Shutting down")
	})
	gsdk.Start()
	gsdk.LogMessage("Before ReadyForPlayers")
	gsdk.ReadyForPlayers()
	gsdk.LogMessage("After ReadyForPlayers")
	time.Sleep(time.Second * 120000)
}
```