# Cluster Manager

## Functionalities
- Send health checks to the replicas
- Expose a http api to manage the cluster: Get nodes, add node etc.

## Limitations
- No authenticaion for connecting to etcd instance

## Requirements
- Etcd v3.5 instance
- All environtmen variables in [Configuration](#Configuartion)

## Configuartion
Configuration keys are stored in environtment variables:

- MEMDB_CLUSTERMANAGER_ETCD_ENDPOINTS. Etcd addresses. Default: 127.0.0.1:2379. Exmaple <address1>,<address2> etc
- MEMDB_CLUSTERMANAGER_AUTH_CLUSTER_KEY. Cluster key required for cluster nodes authentication. Default: 123
- MEMDB_CLUSTERMANAGER_HEALTHCHECK_PERIOD. Time between healthchecks expresed in seconds. Default: 60
- MEMDB_CLUSTERMANAGER_API_PORT. Port
