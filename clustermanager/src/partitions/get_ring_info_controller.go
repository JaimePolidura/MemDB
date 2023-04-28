package partitions

import (
	configuration "clustermanager/src/_shared/config"
	configuration_keys "clustermanager/src/_shared/config/keys"
	"clustermanager/src/partitions/partitions"
	"errors"
	"github.com/labstack/echo/v4"
	"net/http"
)

type GetRingInfoController struct {
	PartitionsRepository partitions.PartitionRepository
	Configuration        *configuration.Configuartion
}

type GetRingInfoResponse struct {
	Entries           partitions.PartitionRingEntries `json:"entries"`
	NodesPerPartition uint32                          `json:"nodesPerPartition"`
	MaxSize           uint32                          `json:"maxSize"`
}

func (controller *GetRingInfoController) GetRingInfo(context echo.Context) error {
	if !controller.Configuration.GetBoolean(configuration_keys.MEMDB_CLUSTERMANAGER_USE_PARTITIONS) {
		return context.JSON(http.StatusBadRequest, errors.New("cannot create partition when MEMDB_CLUSTERMANAGER_USE_PARTITIONS is set to false"))
	}

	nodesPerPartition, err := controller.PartitionsRepository.GetNodesPerPartition()
	ringEntries, err := controller.PartitionsRepository.GetRingEntriesSorted()
	maxSize, err := controller.PartitionsRepository.GetRingMaxSize()

	if err != nil {
		return context.JSON(http.StatusInternalServerError, err)
	}

	return context.JSON(http.StatusOK, GetRingInfoResponse{
		Entries:           ringEntries,
		NodesPerPartition: nodesPerPartition,
		MaxSize:           maxSize,
	})
}
