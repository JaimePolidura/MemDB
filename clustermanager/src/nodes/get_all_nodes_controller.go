package nodes

import (
	"clustermanager/src/_shared/logging"
	"clustermanager/src/nodes/shared"
	"github.com/labstack/echo/v4"
	"net/http"
	"strconv"
)

type GetAllNodesResponse struct {
	Nodes []shared.Node `json:"nodes"`
}

type GetAllNodeController struct {
	NodesRepository shared.NodeRepository
	Logger          *logging.Logger
}

func (controller *GetAllNodeController) GetAllNodes(context echo.Context) error {
	nodes, err := controller.NodesRepository.FindAll()

	if err != nil {
		controller.Logger.Info("Error while executing GetAllNodesController: " + err.Error())
		return err
	}

	controller.Logger.Info("Executed GetAllNodesController responded " + strconv.Itoa(len(nodes)))

	return context.JSON(http.StatusOK, GetAllNodesResponse{
		Nodes: nodes,
	})
}
