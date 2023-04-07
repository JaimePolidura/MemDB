package main

import (
	"clustermanager/src/_shared/config"
	configuration_keys "clustermanager/src/_shared/config/keys"
	"clustermanager/src/_shared/logging"
	"clustermanager/src/_shared/nodes/connection"
	"clustermanager/src/healthchecks"
	"fmt"
	"github.com/labstack/echo/v4"
	clientv3 "go.etcd.io/etcd/client/v3"
)

type ClusterManager struct {
	healthCheckService *healthchecks.HealthCheckService
	etcdNativeClient   *clientv3.Client
	configuration      *configuration.Configuartion
	nodeConnections    *connection.NodeConnections
	api                *echo.Echo
	logger             *logging.Logger
}

func (clusterManager *ClusterManager) start() {
	clusterManager.healthCheckService.Start()

	clusterManager.api.Logger.Fatal(clusterManager.api.Start(":" + clusterManager.configuration.Get(configuration_keys.MEMDB_CLUSTERMANAGER_API_PORT)))
	var input string //Block program
	fmt.Scanln(&input)
}
