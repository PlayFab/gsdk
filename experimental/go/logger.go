package gsdk

import (
	"fmt"
	"io"
	"os"
	"time"

	log "github.com/sirupsen/logrus"
)

func initializeLogger(directory string) {
	utctime := time.Now().UTC().Unix()
	filename := fmt.Sprintf("%s/GSDK_output_%d.txt", directory, utctime)
	f, err := os.OpenFile(filename, os.O_WRONLY|os.O_CREATE|os.O_APPEND, 0666)
	if err != nil {
		panic(err)
	}
	mw := io.MultiWriter(os.Stdout, f)
	log.SetOutput(mw)
}

func logInfo(s string) {
	log.Info(s)
}

func logDebug(s string) {
	log.Debug(s)
}

func logWarn(s string) {
	log.Warn(s)
}

func logError(s string) {
	log.Error(s)
}
