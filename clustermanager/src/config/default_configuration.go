package configuration

import configuration_keys "clustermanager/src/config/keys"

var DEFAULT_CONFIGURATION = map[string]string{
	configuration_keys.MEMDB_CLUSTERMANAGER_HEALTHCHECK_PERIOD: "5",
	configuration_keys.MEMDB_CLUSTERMANAGER_AUTH_CLUSTER_KEY:   "abc",
	configuration_keys.MEMDB_CLUSTERMANAGER_ETCD_ENDPOINTS:     "127.0.0.1:2739", //TODO Used only for dev
}
