package endpoints

import (
	configuration "clustermanager/src/config"
	v2 "clustermanager/src/nodes"
	"clustermanager/src/nodes/messages/request"
	"clustermanager/src/nodes/messages/response"
	"encoding/binary"
	"github.com/labstack/echo/v4"
	"net/http"
	"strconv"
)

type GetClusterConfigController struct {
	Configuration   *configuration.Configuartion
	NodeConnections *v2.ClusterNodeConnections
}

func CreateGetClusterConfigController(configuration *configuration.Configuartion, nodeConnections *v2.ClusterNodeConnections) *GetClusterConfigController {
	return &GetClusterConfigController{
		Configuration:   configuration,
		NodeConnections: nodeConnections,
	}
}

func (this *GetClusterConfigController) GetClusterConfig(context echo.Context) error {
	userAuthKey := this.Configuration.Get(configuration.MEMDB_CLUSTERMANAGER_AUTH_USER_KEY)
	getConfigRequest := request.BuildGetClusterConfigRequest(userAuthKey)

	if res, err := this.NodeConnections.SendRequestToAnySeeder(getConfigRequest); err == nil {
		return context.JSON(http.StatusOK, this.clusterConfigResponseToJSON(res))
	} else {
		return context.JSON(http.StatusBadRequest, "Cannot get cluster configuration from seed nodes: "+err.Error())
	}
}

func (this *GetClusterConfigController) clusterConfigResponseToJSON(response response.Response) GetClusterConfigResponse {
	bytes := []byte(response.ResponseBody)

	nodesPerPartition := binary.BigEndian.Uint32(bytes[:4])
	maxPartitionSize := binary.BigEndian.Uint32(bytes[4:8])
	nNodesInCluster := binary.BigEndian.Uint32(bytes[8:12])
	offset := uint32(12)
	usingPartitions := nodesPerPartition > 0

	ringPositionsByNodeId := make(map[v2.NodeId_t]uint32)
	addressByNodeId := make(map[v2.NodeId_t]string)

	for i := 0; i < int(nNodesInCluster); i++ {
		nodeId := v2.NodeId_t(strconv.Itoa(int(binary.BigEndian.Uint32(bytes[offset : offset+4]))))
		offset += 4

		addressSize := binary.BigEndian.Uint32(bytes[offset : offset+4])
		offset += 4

		address := string(bytes[offset : offset+addressSize])
		offset += addressSize

		addressByNodeId[nodeId] = address
	}
	for i := 0; i < int(nNodesInCluster); i++ {
		nodeId := v2.NodeId_t(strconv.Itoa(int(binary.BigEndian.Uint32(bytes[offset : offset+4]))))
		offset += 4

		ringPosition := binary.BigEndian.Uint32(bytes[offset : offset+4])
		offset += 4

		ringPositionsByNodeId[nodeId] = ringPosition
	}

	nodes := make([]NodeResponse, nNodesInCluster)
	for nodeId, address := range addressByNodeId {
		ringPosition := uint32(0)
		if usingPartitions {
			ringPosition = ringPositionsByNodeId[nodeId]
		}

		nodes = append(nodes, NodeResponse{
			NodeId:       nodeId,
			Address:      address,
			RingPosition: ringPosition,
		})
	}

	return GetClusterConfigResponse{
		NodesPerPartition: nodesPerPartition,
		MaxPartitionSize:  maxPartitionSize,
		Nodes:             nodes,
	}
}

type GetClusterConfigResponse struct {
	NodesPerPartition uint32         `json:"nodesPerPartition"`
	MaxPartitionSize  uint32         `json:"maxPartitionSize"`
	Nodes             []NodeResponse `json:"nodes"`
}

type NodeResponse struct {
	NodeId       v2.NodeId_t `json:"nodeId"`
	RingPosition uint32      `json:"ringPosition"`
	Address      string      `json:"address"`
}
