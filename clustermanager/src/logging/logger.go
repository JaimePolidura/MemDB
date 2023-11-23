package logging

import (
	"clustermanager/src/config"
	"log"
)

type Logger struct {
	Configuration *config.Configuartion
	NativeLogger  *log.Logger
}

func (client *Logger) Info(text string) {
	if client.Configuration.GetBoolean(config.MEMDB_CLUSTERMANAGER_DO_LOGGING) {
		client.NativeLogger.Println(text)
	}
}

func (client *Logger) Error(text string) {
	if client.Configuration.GetBoolean(config.MEMDB_CLUSTERMANAGER_DO_LOGGING) {
		client.NativeLogger.Fatal(text)
	}
}
