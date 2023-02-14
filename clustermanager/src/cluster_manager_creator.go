package main

import (
	"clustermanager/src/_shared/config"
	"clustermanager/src/_shared/config/keys"
	"clustermanager/src/_shared/etcd"
	nodes2 "clustermanager/src/_shared/nodes"
	"clustermanager/src/_shared/nodes/connection"
	"clustermanager/src/api/login"
	"clustermanager/src/healthchecks"
	echojwt "github.com/labstack/echo-jwt"
	"github.com/labstack/echo/v4"
	"github.com/labstack/echo/v4/middleware"
	clientv3 "go.etcd.io/etcd/client/v3"
	"strings"
	"time"
)

func CreateClusterManager() *ClusterManager {
	loadedConfiguration := configuration.LoadConfiguration()
	etcdNativeClient := createEtcdNativeClient(loadedConfiguration)
	nodeConnections := connection.CreateNodeConnectionsObject()
	healthService := createHealthCheckService(loadedConfiguration, nodeConnections, etcdNativeClient)
	apiEcho := configureHttpApi(loadedConfiguration)

	return &ClusterManager{
		configuration:      loadedConfiguration,
		etcdNativeClient:   etcdNativeClient,
		healthCheckService: healthService,
		nodeConnections:    nodeConnections,
		api:                apiEcho,
	}
}

func createEtcdNativeClient(configuration *configuration.Configuartion) *clientv3.Client {
	client, err := clientv3.New(clientv3.Config{
		Endpoints:   strings.Split(configuration.Get(configuration_keys.MEMDB_CLUSTERMANAGER_ETCD_ENDPOINTS), ","),
		DialTimeout: time.Second * 30,
	})

	if err != nil {
		panic("Cannot connect to etcd: " + err.Error())
	}

	return client
}

func createHealthCheckService(configuration *configuration.Configuartion,
	connections *connection.NodeConnections,
	etcdNativeClient *clientv3.Client) *healthchecks.HealthCheckService {

	customEtcdClient := &etcd.EtcdClient[nodes2.Node]{NativeClient: etcdNativeClient, Timeout: time.Second * 30}
	nodesRepository := nodes2.EtcdNodeRepository{Client: customEtcdClient}

	return &healthchecks.HealthCheckService{
		NodesRespository: nodesRepository,
		Configuration:    configuration,
		NodeConnections:  connections,
	}
}

func configureHttpApi(configuration *configuration.Configuartion) *echo.Echo {
	echoApi := echo.New()
	echoApi.HideBanner = true
	echoApi.Use(middleware.Recover())

	apiGroup := echoApi.Group("/api")
	apiGroup.Use(echojwt.WithConfig(echojwt.Config{
		SigningKey: configuration.Get(configuration_keys.MEMDB_CLUSTERMANAGER_API_SECRET_KEY),
	}))

	loginController := &login.LoginController{Configuration: configuration}

	echoApi.POST("/login", loginController.Login)

	return echoApi
}
