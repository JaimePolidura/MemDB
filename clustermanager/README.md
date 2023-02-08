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

- MEMDB_CLUSTERMANAGER_ETCD_ENDPOINTS. Required. Etcd addresses. Exmaple <address1>,<address2> etc
- MEMDB_CLUSTERMANAGER_HEALTHCHECK_PERIOD. Optional. Time per healthchecks expresed in seconds