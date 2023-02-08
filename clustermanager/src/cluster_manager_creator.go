package main

import (
	"clustermanager/src/_shared/etcd"
	configuration "clustermanager/src/config"
	configuration_keys "clustermanager/src/config/keys"
	"clustermanager/src/healthchecks"
	"clustermanager/src/nodes"
	clientv3 "go.etcd.io/etcd/client/v3"
	"strings"
	"time"
)

func CreateClusterManager() *ClusterManager {
	loadedConfiguration := loadConfiguration()
	etcdNativeClient := createEtcdNativeClient(loadedConfiguration)
	healthService := createHealthCheckService(loadedConfiguration, etcdNativeClient)

	return &ClusterManager{configuration: loadedConfiguration, etcdNativeClient: etcdNativeClient, healthCheckService: healthService}
}

func loadConfiguration() *configuration.Configuartion {
	return &configuration.Configuartion{}
}

func createEtcdNativeClient(configuration *configuration.Configuartion) *clientv3.Client {
	client, err := clientv3.New(clientv3.Config{
		Endpoints:   strings.Split(configuration.Get(configuration_keys.MEMDB_CLUSTERMANAGER_ETCD_ENDPOINTS), ","),
		DialTimeout: time.Second * 30,
	})

	if err != nil {
		panic("Cannot connect to etcd")
	}

	return client
}

func createHealthCheckService(configuration *configuration.Configuartion, etcdNativeClient *clientv3.Client) *healthchecks.HealthCheckService {
	customEtcdClient := &etcd.EtcdClient[nodes.Node]{NativeClient: etcdNativeClient, Timeout: time.Second * 30}
	nodesRepository := nodes.EtcdNodeRepository{Client: customEtcdClient}

	return &healthchecks.HealthCheckService{NodesRespository: nodesRepository, Configuration: configuration}
}
