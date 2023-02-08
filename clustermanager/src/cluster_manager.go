package main

import (
	configuration "clustermanager/src/config"
	clientv3 "go.etcd.io/etcd/client/v3"
)

type ClusterManager struct {
	etcdClientConnection *clientv3.Client
	configuration        *configuration.Configuartion
}

func (clusterManager *ClusterManager) start() {

}
