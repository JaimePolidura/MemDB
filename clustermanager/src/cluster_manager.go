package main

import (
	configuration "clustermanager/src/config"
	"clustermanager/src/healthchecks"
	clientv3 "go.etcd.io/etcd/client/v3"
)

type ClusterManager struct {
	healthCheckService *healthchecks.HealthCheckService
	etcdNativeClient   *clientv3.Client
	configuration      *configuration.Configuartion
}

func (clusterManager *ClusterManager) start() {
	clusterManager.healthCheckService.Start()
}
