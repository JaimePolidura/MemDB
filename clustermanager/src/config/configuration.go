package config

import (
	"os"
	"strconv"
	"strings"
	"sync"
)

type Configuartion struct {
	cachedConfigurationKeys *sync.Map
}

func (this *Configuartion) GetBoolean(key string) bool {
	return this.Get(key) == "true"
}

func (this *Configuartion) GetInt(key string) int64 {
	strValue := this.Get(key)
	intValue, err := strconv.ParseInt(strValue, 10, 64)

	if err != nil {
		panic("Configuartion value " + key + " incorrect format. Expected an integer")
	}

	return intValue
}

func (this *Configuartion) GetConfig(key string) []string {
	rawArray := this.Get(key)
	return strings.Split(rawArray, ",")
}

func (this *Configuartion) Get(key string) string {
	if cachedValue, inCache := this.cachedConfigurationKeys.Load(key); inCache {
		return cachedValue.(string)
	}

	envValue, envExists := os.LookupEnv(key)
	defaultValue, defaultValueExists := DEFAULT_CONFIGURATION[key]

	if !envExists && !defaultValueExists {
		panic("Environtment variable " + key + " missing")
	}

	if envExists {
		this.cachedConfigurationKeys.Store(key, envValue)
		return envValue
	}

	this.cachedConfigurationKeys.Store(key, defaultValue)
	return defaultValue
}

func LoadConfiguration(args []string) *Configuartion {
	return &Configuartion{cachedConfigurationKeys: argsToConfigMap(args)}
}

func argsToConfigMap(args []string) *sync.Map {
	argMap := &sync.Map{}

	for i := 1; i < len(args); i++ {
		arg := args[i]
		splitedByEq := strings.Split(arg, "=")
		key := splitedByEq[0]
		value := splitedByEq[1]

		argMap.Store(key, value)
	}

	return argMap
}
