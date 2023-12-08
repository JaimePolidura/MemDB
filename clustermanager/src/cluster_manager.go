package main

import (
	"clustermanager/src/config"
	"clustermanager/src/logging"
	"fmt"
	"github.com/labstack/echo/v4"
)

type ClusterManager struct {
	configuration *config.Configuartion
	api           *echo.Echo
	logger        *logging.Logger
}

func (clusterManager *ClusterManager) start() {
	clusterManager.api.Logger.Fatal(clusterManager.api.Start(":" + clusterManager.configuration.Get(config.API_PORT)))
	var input string //Block program
	fmt.Scanln(&input)
}
