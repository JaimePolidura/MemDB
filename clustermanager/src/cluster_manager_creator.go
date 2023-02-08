package main

import (
	configuration "clustermanager/src/config"
	configuration_keys "clustermanager/src/config/keys"
	clientv3 "go.etcd.io/etcd/client/v3"
	"strings"
	"time"
)

func CreateClusterManager() *ClusterManager {
	loadedConfiguration := loadConfiguration()
	etcdClient := createEtcdNodeConnection(loadedConfiguration)

	return &ClusterManager{configuration: loadedConfiguration, etcdClientConnection: etcdClient}
}

func loadConfiguration() *configuration.Configuartion {
	return &configuration.Configuartion{}
}

func createEtcdNodeConnection(configuration *configuration.Configuartion) *clientv3.Client {
	client, err := clientv3.New(clientv3.Config{
		Endpoints:   strings.Split(configuration.Get(configuration_keys.MEMDB_CLUSTERMANAGER_ETCD_ENDPOINTS), ","),
		DialTimeout: time.Minute,
	})

	if err != nil {
		panic("Cannot connect to etcd")
	}

	return client
}
