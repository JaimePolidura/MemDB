package configuration

import configuration_keys "clustermanager/src/config/keys"

var DEFAULT_CONFIGURATION = map[string]string{
	configuration_keys.MEMDB_CLUSTERMANAGER_HEALTHCHECK_PERIOD: "30",
	configuration_keys.MEMDB_CLUSTERMANAGER_AUTH_CLUSTER_KEY:   "abc",
}
