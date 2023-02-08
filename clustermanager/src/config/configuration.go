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

	envValue, envExists := os.LookupEnv(key)

	if !envExists {
		panic("Environtment variable " + key + " missing")
	}

	configuartion.cachedConfigurationKeys[key] = envValue

	return envValue
}

func (configuartion *Configuartion) GetOrDefaultInt(key string, defaultIfNotFound int64) int64 {
	strValue := configuartion.GetOrDefault(key, strconv.FormatInt(defaultIfNotFound, 10))
	intValue, err := strconv.ParseInt(strValue, 10, 64)

	if err != nil {
		panic("Configuartion value " + key + " incorrect format. Expected an integer")
	}

	return intValue
}

func (configuartion *Configuartion) GetOrDefault(key string, defaultIfNotFound string) string {
	if cachedValue, inCache := configuartion.cachedConfigurationKeys[key]; inCache {
		return cachedValue
	}

	envValue, envExists := os.LookupEnv(key)
	if !envExists {
		configuartion.cachedConfigurationKeys[key] = defaultIfNotFound
		return defaultIfNotFound
	}

	configuartion.cachedConfigurationKeys[key] = defaultIfNotFound

	return envValue
}
