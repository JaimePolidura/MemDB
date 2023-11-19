package logging

import (
	"clustermanager/src/config"
	"clustermanager/src/config/keys"
	"log"
)

type Logger struct {
	Configuration *configuration.Configuartion
	NativeLogger  *log.Logger
}

func (client *Logger) Info(text string) {
	if client.Configuration.GetBoolean(configuration_keys.MEMDB_CLUSTERMANAGER_DO_LOGGING) {
		client.NativeLogger.Println(text)
	}
}

func (client *Logger) Error(text string) {
	if client.Configuration.GetBoolean(configuration_keys.MEMDB_CLUSTERMANAGER_DO_LOGGING) {
		client.NativeLogger.Fatal(text)
	}
}
