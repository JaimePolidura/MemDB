package configuration

import (
	"clustermanager/src/_shared/config/keys"
)

var DEFAULT_CONFIGURATION = map[string]string{
	configuration_keys.MEMDB_CLUSTERMANAGER_HEALTHCHECK_PERIOD: "60",
	configuration_keys.MEMDB_CLUSTERMANAGER_AUTH_CLUSTER_KEY:   "abc",
	configuration_keys.MEMDB_CLUSTERMANAGER_ETCD_ENDPOINTS:     "127.0.0.1:2379",
	configuration_keys.MEMDB_CLUSTERMANAGER_API_PORT:           "8080",
	configuration_keys.MEMDB_CLUSTERMANAGER_API_SECRET_KEY:     "def",
	configuration_keys.MEMDB_CLUSTERMANAGER_DO_LOGGING:         "false",
}
