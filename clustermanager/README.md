# Cluster Manager

- Send health checks to cluster nodes
- Expose a HTTP API to manage the cluster.

## Limitations
- No authenticaion to connection etcd instance

## Configuartion
Configuration keys are stored in environtment variables:

- MEMDB_CLUSTERMANAGER_ETCD_ENDPOINTS. Etcd addresses. Default: 127.0.0.1:2379. Exmaple <address1>,<address2> etc
- MEMDB_CLUSTERMANAGER_HEALTHCHECK_PERIOD. Time between healthchecks expresed in seconds. Default: 60
- MEMDB_CLUSTERMANAGER_API_PORT. Port of HTTP API. Default: 8080
- MEMDB_CLUSTERMANAGER_DO_LOGGING. If set to true, it will log request, healthchecks etc. Default: false
- 
- MEMDB_CLUSTERMANAGER_API_JWT_SECRET_KEY. Secret key used for authentication for the HTTP API. Default: abc
- MEMDB_CLUSTERMANAGER_AUTH_MAINTENANCE_KEY. Maintenance key required for cluster nodes authentication, usecases: healthcheck. Default: 456
- MEMDB_CLUSTERMANAGER_AUTH_API_KEY. External users who want to use the cluster manager API or connect directly to the nodes, should use this key. Default: 789
