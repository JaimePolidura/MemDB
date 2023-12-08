package main

import (
	"clustermanager/src/config"
	"clustermanager/src/endpoints"
	"clustermanager/src/logging"
	"clustermanager/src/nodes"
	echojwt "github.com/labstack/echo-jwt"
	"github.com/labstack/echo/v4"
	"github.com/labstack/echo/v4/middleware"
	"log"
	"os"
)

func CreateClusterManager(args []string) *ClusterManager {
	loadedConfiguration := config.LoadConfiguration(args)
	logger := &logging.Logger{
		NativeLogger:  log.New(os.Stdout, "[ClusterManager] ", log.Ldate|log.Ltime|log.Lmsgprefix),
		Configuration: loadedConfiguration,
	}
	nodeConnections := nodes.CreateClusterNodeConnections(loadedConfiguration)
	apiEcho := configureHttpApi(loadedConfiguration, nodeConnections, logger)

	return &ClusterManager{
		configuration: loadedConfiguration,
		api:           apiEcho,
		logger:        logger,
	}
}

func configureHttpApi(configuration *config.Configuartion,
	connections *nodes.ClusterNodeConnections,
	logger *logging.Logger) *echo.Echo {

	echoApi := echo.New()
	echoApi.HideBanner = true
	echoApi.Use(middleware.Recover())

	apiGroup := echoApi.Group("/api")
	apiGroup.Use(echojwt.WithConfig(echojwt.Config{
		SigningKey: []byte(configuration.Get(config.API_JWT_SECRET_KEY)),
	}))

	getClusterConfigController := endpoints.CreateGetClusterConfigController(configuration, connections)
	removeNodeClusterController := endpoints.CreateRemoveNodeClusterController(configuration, connections)
	loginController := endpoints.CreateLoginController(configuration, logger)

	apiGroup.DELETE("/cluster/nodes/:nodeId", removeNodeClusterController.Remove)
	apiGroup.GET("/cluster/config", getClusterConfigController.GetClusterConfig)
	echoApi.POST("/login", loginController.Login)

	return echoApi
}
