package main

import (
	configuration "clustermanager/src/_shared/config"
	configuration_keys "clustermanager/src/_shared/config/keys"
	"clustermanager/src/_shared/etcd"
	"clustermanager/src/_shared/logging"
	"clustermanager/src/_shared/utils"
	"clustermanager/src/healthchecks"
	nodes2 "clustermanager/src/nodes"
	"clustermanager/src/nodes/nodes"
	"clustermanager/src/nodes/nodes/connection"
	"clustermanager/src/partitions"
	partitions2 "clustermanager/src/partitions/partitions"
	"crypto/md5"
	echojwt "github.com/labstack/echo-jwt"
	"github.com/labstack/echo/v4"
	"github.com/labstack/echo/v4/middleware"
	clientv3 "go.etcd.io/etcd/client/v3"
	"log"
	"os"
	"strings"
	"time"
)

func CreateClusterManager(args []string) *ClusterManager {
	loadedConfiguration := configuration.LoadConfiguration(args)
	logger := &logging.Logger{
		NativeLogger:  log.New(os.Stdout, "[ClusterManager] ", log.Ldate|log.Ltime|log.Lmsgprefix),
		Configuration: loadedConfiguration,
	}
	etcdNativeClient := createEtcdNativeClient(loadedConfiguration)
	partitionsRepository := &partitions2.PartitionRepository{Client: etcd.EtcdClient[string]{NativeClient: etcdNativeClient, Timeout: time.Second * 30}, Logger: logger}
	nodesRepository := &nodes.NodeRepository{Client: etcd.EtcdClient[nodes.Node]{NativeClient: etcdNativeClient, Timeout: time.Second * 30}}
	nodeConnections := connection.CreateNodeConnectionsObject(logger)
	healthService := createHealthCheckService(loadedConfiguration, nodeConnections, nodesRepository, logger)
	apiEcho := configureHttpApi(loadedConfiguration, logger, partitionsRepository, nodesRepository)

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
	nodesRepository *nodes.NodeRepository,
	logger *logging.Logger) *healthchecks.HealthCheckService {

	return &healthchecks.HealthCheckService{
		NodesRespository: nodesRepository,
		Configuration:    configuration,
		NodeConnections:  connections,
		Logger:           logger,
	}
}

func configureHttpApi(configuration *configuration.Configuartion,
	logger *logging.Logger,
	partitionRepository *partitions2.PartitionRepository,
	nodesRepository *nodes.NodeRepository) *echo.Echo {

	echoApi := echo.New()
	echoApi.HideBanner = true
	echoApi.Use(middleware.Recover())

	apiGroup := echoApi.Group("/api")
	apiGroup.Use(echojwt.WithConfig(echojwt.Config{
		SigningKey: []byte(configuration.Get(configuration_keys.MEMDB_CLUSTERMANAGER_API_JWT_SECRET_KEY)),
	}))

	ringNodeAllocator := &partitions.RingNodeAllocator{HashCalculator: utils.HashCalculator{HashAlgorithm: md5.New()}, PartitionsRepository: partitionRepository}

	createNodeController := &nodes2.CreateNodeController{NodesRepository: nodesRepository, RingNodeAllocator: ringNodeAllocator, Configuration: configuration}
	getAllNodesController := nodes2.GetAllNodeController{NodesRepository: nodesRepository, Logger: logger, PartitionRepository: partitionRepository}
	loginController := &nodes2.LoginController{Configuration: configuration, Logger: logger}
	getRingController := &partitions.GetRingInfoController{PartitionsRepository: partitionRepository, Configuration: configuration}

	echoApi.POST("/login", loginController.Login)

	apiGroup.POST("/nodes/create", createNodeController.CreateNode)
	apiGroup.GET("/nodes/all", getAllNodesController.GetAllNodes)
	apiGroup.GET("/partitions/ring/info", getRingController.GetRingInfo)

	return echoApi
}
