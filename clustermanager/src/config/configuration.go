package configuration

import (
	"os"
	"strconv"
)

type Configuartion struct {
	cachedConfigurationKeys map[string]string
}

func (configuartion *Configuartion) Get(key string) string {
	if cachedValue, inCache := configuartion.cachedConfigurationKeys[key]; inCache {
		return cachedValue
	}
	if defaultValue, exist := DEFAULT_CONFIGURATION[key]; exist {
		configuartion.cachedConfigurationKeys[key] = defaultValue
		return defaultValue
	}

	envValue, envExists := os.LookupEnv(key)

	if !envExists {
		panic("Environtment variable " + key + " missing")
	}

	configuartion.cachedConfigurationKeys[key] = envValue

	return envValue
}

func (configuartion *Configuartion) GetInt(key string) int64 {
	strValue := configuartion.Get(key)
	intValue, err := strconv.ParseInt(strValue, 10, 64)

	if err != nil {
		panic("Configuartion value " + key + " incorrect format. Expected an integer")
	}

	return intValue
}
