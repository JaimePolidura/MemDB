# Cluster Manager

## Functionalities
- Send health checks to the replicas
- Expose a http api to manage the cluster: Get nodes, add node etc.

## Requirements
- Etcd v3.5 instance
- All environtmen variables in [Configuration](#Configuartion) 

## Configuartion
Configuration keys are stored in environtment variables:

- MEMDB_CLUSTERMANAGER_ETCD_ENDPOINTS Contains etcd addresses. Exmaple <address1>,<address2> etc

## Limitations
- No authenticaion for connecting to etcd instance