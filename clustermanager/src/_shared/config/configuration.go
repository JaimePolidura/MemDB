package configuration

import (
	"os"
	"strconv"
	"sync"
)

type Configuartion struct {
	cachedConfigurationKeys sync.Map
}

func (configuartion *Configuartion) GetBoolean(key string) bool {
	return configuartion.Get(key) == "true"
}

func (configuartion *Configuartion) GetInt(key string) int64 {
	strValue := configuartion.Get(key)
	intValue, err := strconv.ParseInt(strValue, 10, 64)

	if err != nil {
		panic("Configuartion value " + key + " incorrect format. Expected an integer")
	}

	return intValue
}

func (configuartion *Configuartion) Get(key string) string {
	if cachedValue, inCache := configuartion.cachedConfigurationKeys.Load(key); inCache {
		return cachedValue.(string)
	}

	envValue, envExists := os.LookupEnv(key)
	defaultValue, defaultValueExists := DEFAULT_CONFIGURATION[key]

	if !envExists && !defaultValueExists {
		panic("Environtment variable " + key + " missing")
	}

	if envExists {
		configuartion.cachedConfigurationKeys.Store(key, envValue)
		return envValue
	}

	configuartion.cachedConfigurationKeys.Store(key, defaultValue)
	return defaultValue
}

func LoadConfiguration() *Configuartion {
	return &Configuartion{cachedConfigurationKeys: sync.Map{}}
}
