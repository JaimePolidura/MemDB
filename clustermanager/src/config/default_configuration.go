package configuration

import (
	"clustermanager/src/config/keys"
)

var DEFAULT_CONFIGURATION = map[string]string{
	configuration_keys.MEMDB_CLUSTERMANAGER_HEALTHCHECK_PERIOD:                "1000060",
	configuration_keys.MEMDB_CLUSTERMANAGER_AUTH_MAINTENANCE_KEY:              "456",
	configuration_keys.MEMDB_CLUSTERMANAGER_AUTH_API_KEY:                      "789",
	configuration_keys.MEMDB_CLUSTERMANAGER_API_JWT_SECRET_KEY:                "abc",
	configuration_keys.configuration_keys.MEMDB_CLUSTERMANAGER_ETCD_ENDPOINTS: "127.0.0.1:2379",
	configuration_keys.MEMDB_CLUSTERMANAGER_API_PORT:                          "8080",
	configuration_keys.MEMDB_CLUSTERMANAGER_DO_LOGGING:                        "false",
	configuration_keys.MEMDB_CLUSTERMANAGER_USE_PARTITIONS:                    "false",
}
