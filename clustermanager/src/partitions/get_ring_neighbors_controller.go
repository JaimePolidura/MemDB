package partitions

import (
	configuration "clustermanager/src/_shared/config"
	configuration_keys "clustermanager/src/_shared/config/keys"
	"clustermanager/src/nodes/nodes"
	"clustermanager/src/partitions/partitions"
	"errors"
	"github.com/labstack/echo/v4"
	"net/http"
)

type GetRingNeighborsController struct {
	PartitionsRepository *partitions.PartitionRepository
	Configuration        *configuration.Configuartion
	NodeRepository       *nodes.NodeRepository
}

type GetRingNeighborsResponse struct {
	Neighbors []nodes.Node `json:"neighbors"`
}

func (controller *GetRingNeighborsController) GetRingNeighbors(context echo.Context) error {
	if !controller.Configuration.GetBoolean(configuration_keys.MEMDB_CLUSTERMANAGER_USE_PARTITIONS) {
		return context.JSON(http.StatusBadRequest, errors.New(("cannot create partition when MEMDB_CLUSTERMANAGER_USE_PARTITIONS is set to false")))
	}

	nodeId := nodes.NodeId_t(context.QueryParam("nodeId"))
	nodesPerPartition, err := controller.PartitionsRepository.GetNodesPerPartition()
	ringEntries, err := controller.PartitionsRepository.GetRingEntriesSorted()

	if err != nil {
		return context.JSON(http.StatusBadRequest, err)
	}

	neighborsRingEntries, found := ringEntries.GetNeighborsByNodeId(nodeId, nodesPerPartition)
	if !found {
		return context.JSON(http.StatusBadRequest, "node not found")
	}

	nodesEntries := make([]nodes.Node, len(neighborsRingEntries)+1)
	for i, ringEntry := range neighborsRingEntries {
		node, _ := controller.NodeRepository.FindById(ringEntry.NodeId)
		nodesEntries[i] = node
	}

	selfNode, _ := controller.NodeRepository.FindById(nodeId)
	nodesEntries[len(nodesEntries)-1] = selfNode

	return context.JSON(http.StatusOK, GetRingNeighborsResponse{
		Neighbors: nodesEntries,
	})
}
