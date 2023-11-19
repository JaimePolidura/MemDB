package nodes

import (
	"clustermanager/src/config"
	"clustermanager/src/config/keys"
	"clustermanager/src/logging"
	"clustermanager/src/nodes/nodes"
	partitions2 "clustermanager/src/partitions/partitions"
	"github.com/labstack/echo/v4"
	"net/http"
	"strconv"
)

type GetAllNodesResponse struct {
	Nodes []nodes.Node `json:"nodes"`
}

type GetAllNodeController struct {
	PartitionRepository *partitions2.PartitionRepository
	Configuartion       configuration.Configuartion
	NodesRepository     *nodes.NodeRepository
	Logger              *logging.Logger
}

func (controller *GetAllNodeController) GetAllNodes(context echo.Context) error {
	var nodes []nodes.Node
	var err error
	nodeId := context.QueryParam("nodeId")

	if controller.Configuartion.GetBoolean(configuration_keys.MEMDB_CLUSTERMANAGER_USE_PARTITIONS) {
		nodes, err = controller.getNodesNeighborsFromPartition(nodeId)
	} else {
		nodes, err = controller.NodesRepository.FindAll()
	}

	if err != nil {
		controller.Logger.Info("Error while executing GetAllNodesController: " + err.Error())
		return err
	}

	controller.Logger.Info("Executed GetAllNodesController responded " + strconv.Itoa(len(nodes)))

	return context.JSON(http.StatusOK, GetAllNodesResponse{
		Nodes: nodes,
	})
}

func (controller *GetAllNodeController) getNodesNeighborsFromPartition(nodeIdString string) ([]nodes.Node, error) {
	selfNodeId := nodes.NodeId_t(nodeIdString)
	selfNode, err := controller.NodesRepository.FindById(selfNodeId)

	ringEntries, err := controller.PartitionRepository.GetRingEntriesSorted()
	nodesPerPartition, err := controller.PartitionRepository.GetNodesPerPartition()
	if err != nil {
		return []nodes.Node{}, err
	}

	var nodes []nodes.Node
	neighborsEntries, _ := ringEntries.GetNeighborsByNodeId(selfNodeId, nodesPerPartition-1)

	for _, entry := range neighborsEntries {
		node, _ := controller.NodesRepository.FindById(entry.NodeId)
		nodes = append(nodes, node)
	}

	return append(nodes, selfNode), nil
}
