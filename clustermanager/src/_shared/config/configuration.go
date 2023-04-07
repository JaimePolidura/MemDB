package configuration

import (
	"os"
	"strconv"
)

type Configuartion struct {
	cachedConfigurationKeys map[string]string
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
	if cachedValue, inCache := configuartion.cachedConfigurationKeys[key]; inCache {
		return cachedValue
	}

	envValue, envExists := os.LookupEnv(key)
	defaultValue, defaultValueExists := DEFAULT_CONFIGURATION[key]
	
	if !envExists && !defaultValueExists {
		panic("Environtment variable " + key + " missing")
	}

	if envExists {
		configuartion.cachedConfigurationKeys[key] = envValue
		return envValue
	}

	configuartion.cachedConfigurationKeys[key] = defaultValue
	return defaultValue
}

func LoadConfiguration() *Configuartion {
	return &Configuartion{cachedConfigurationKeys: make(map[string]string)}
}
