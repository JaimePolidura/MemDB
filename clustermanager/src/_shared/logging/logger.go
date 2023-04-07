package logging

import (
	configuration "clustermanager/src/_shared/config"
	configuration_keys "clustermanager/src/_shared/config/keys"
	"log"
)

type Logger struct {
	Configuration *configuration.Configuartion
	NativeLogger  *log.Logger
}

func (client *Logger) Log(text string) {
	if client.Configuration.GetBoolean(configuration_keys.MEMDB_CLUSTERMANAGER_DO_LOGGING) {
		client.NativeLogger.Println(text)
	}
}
