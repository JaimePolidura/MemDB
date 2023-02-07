package configuration

import (
	"os"
)

type Configuartion struct {
	cachedConfigurationKeys map[string]string
}

func (configuartion *Configuartion) Get(key string) (string, error) {
	if cachedValue, inCache := configuartion.cachedConfigurationKeys[key]; inCache {
		return cachedValue, nil
	}

	envValue, envExists := os.LookupEnv(key)

	if !envExists {
		panic("Environtment variable " + key + " missing")
	}

	configuartion.cachedConfigurationKeys[key] = envValue

	return envValue, nil
}
