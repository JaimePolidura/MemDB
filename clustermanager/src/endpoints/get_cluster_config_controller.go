package endpoints

import configuration "clustermanager/src/config"

type GetClusterConfigController struct {
	Configuration *configuration.Configuartion
}

type GetClusterConfigResponse struct {
	nodesPerPartition uint32
	maxPartitionSize  uint32
}

type ClusterNode struct {
}
