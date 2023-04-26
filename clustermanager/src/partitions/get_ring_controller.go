package partitions

import (
	configuration "clustermanager/src/_shared/config"
	configuration_keys "clustermanager/src/_shared/config/keys"
	"clustermanager/src/partitions/partitions"
	"errors"
	"github.com/labstack/echo/v4"
	"net/http"
)

type GetRingController struct {
	PartitionsRepository partitions.PartitionRepository
	Configuration        *configuration.Configuartion
}

func (controller *GetRingController) GetRing(context echo.Context) error {
	if !controller.Configuration.GetBoolean(configuration_keys.MEMDB_CLUSTERMANAGER_USE_PARTITIONS) {
		return context.JSON(http.StatusBadRequest, errors.New("cannot create partition when MEMDB_CLUSTERMANAGER_USE_PARTITIONS is set to false"))
	}

	ring, err := controller.PartitionsRepository.GetRing()

	if err != nil {
		return context.JSON(http.StatusInternalServerError, err)
	} else {
		return context.JSON(http.StatusOK, ring)
	}
}
