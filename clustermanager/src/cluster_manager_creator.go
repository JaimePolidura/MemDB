package main

import (
	"clustermanager/src/_shared/config"
	"clustermanager/src/_shared/config/keys"
	"clustermanager/src/_shared/etcd"
	"clustermanager/src/_shared/logging"
	"clustermanager/src/healthchecks"
	nodes2 "clustermanager/src/nodes"
	"clustermanager/src/nodes/nodes"
	"clustermanager/src/nodes/nodes/connection"
	"clustermanager/src/partitions"
	partitions2 "clustermanager/src/partitions/partitions"
	echojwt "github.com/labstack/echo-jwt"
	"github.com/labstack/echo/v4"
	"github.com/labstack/echo/v4/middleware"
	clientv3 "go.etcd.io/etcd/client/v3"
	"log"
	"os"
	"strings"
	"time"
)

func CreateClusterManager() *ClusterManager {
	loadedConfiguration := configuration.LoadConfiguration()
	logger := &logging.Logger{
		NativeLogger:  log.New(os.Stdout, "[ClusterManager] ", log.Ldate|log.Ltime|log.Lmsgprefix),
		Configuration: loadedConfiguration,
	}
	etcdNativeClient := createEtcdNativeClient(loadedConfiguration)
	nodeConnections := connection.CreateNodeConnectionsObject(logger)
	healthService := createHealthCheckService(loadedConfiguration, nodeConnections, etcdNativeClient, logger)
	apiEcho := configureHttpApi(loadedConfiguration, etcdNativeClient, logger)

	return &ClusterManager{
		configuration:      loadedConfiguration,
		etcdNativeClient:   etcdNativeClient,
		healthCheckService: healthService,
		nodeConnections:    nodeConnections,
		api:                apiEcho,
		logger:             logger,
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

func createHealthCheckService(
	configuration *configuration.Configuartion,
	connections *connection.NodeConnections,
	etcdNativeClient *clientv3.Client,
	logger *logging.Logger) *healthchecks.HealthCheckService {

	customEtcdClient := &etcd.EtcdClient[nodes.Node]{NativeClient: etcdNativeClient, Timeout: time.Second * 30}
	nodesRepository := nodes.EtcdNodeRepository{Client: customEtcdClient}

	return &healthchecks.HealthCheckService{
		NodesRespository: nodesRepository,
		Configuration:    configuration,
		NodeConnections:  connections,
		Logger:           logger,
	}
}

func configureHttpApi(configuration *configuration.Configuartion, etcdNativeClient *clientv3.Client, logger *logging.Logger) *echo.Echo {
	echoApi := echo.New()
	echoApi.HideBanner = true
	echoApi.Use(middleware.Recover())

	apiGroup := echoApi.Group("/api")
	apiGroup.Use(echojwt.WithConfig(echojwt.Config{
		SigningKey: []byte(configuration.Get(configuration_keys.MEMDB_CLUSTERMANAGER_API_JWT_SECRET_KEY)),
	}))

	partitionsRepository := partitions2.EtcdPartitionRepository{Client: &etcd.EtcdClient[string]{NativeClient: etcdNativeClient, Timeout: time.Second * 30}}
	nodesRepository := nodes.EtcdNodeRepository{Client: &etcd.EtcdClient[nodes.Node]{NativeClient: etcdNativeClient, Timeout: time.Second * 30}}

	getAllNodesController := nodes2.GetAllNodeController{NodesRepository: nodesRepository, Logger: logger}
	loginController := &nodes2.LoginController{Configuration: configuration, Logger: logger}
	createNodeController := &nodes2.CreateNodeController{NodesRepository: nodesRepository}
	getRingController := partitions.GetRingController{PartitionsRepository: partitionsRepository, Configuration: configuration}

	echoApi.POST("/login", loginController.Login)

	apiGroup.POST("/nodes/create", createNodeController.CreateNode)
	apiGroup.GET("/nodes/all", getAllNodesController.GetAllNodes)
	apiGroup.GET("/partitions/ring", getRingController.GetRing)

	return echoApi
}
