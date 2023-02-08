package main

import (
	configuration "clustermanager/src/config"
	"clustermanager/src/healthchecks"
	"clustermanager/src/nodes/connection"
	"fmt"
	clientv3 "go.etcd.io/etcd/client/v3"
)

type ClusterManager struct {
	healthCheckService *healthchecks.HealthCheckService
	etcdNativeClient   *clientv3.Client
	configuration      *configuration.Configuartion
	nodeConnections    *connection.NodeConnections
}

func (clusterManager *ClusterManager) start() {
	clusterManager.healthCheckService.Start()

	var input string //Block program
	fmt.Scanln(&input)
}
