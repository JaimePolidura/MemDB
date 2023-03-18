package api

import (
	"clustermanager/src/_shared/nodes"
	"github.com/labstack/echo/v4"
	"net/http"
)

type GetAllNodesResponse struct {
	Nodes []nodes.Node `json:"nodes"`
}

type GetAllNodeController struct {
	NodesRepository nodes.NodeRepository
}

func (controller *GetAllNodeController) GetAllNodes(context echo.Context) error {
	nodes, err := controller.NodesRepository.FindAll()

	if err != nil {
		return err
	}

	return context.JSON(http.StatusOK, GetAllNodesResponse{
		Nodes: nodes,
	})
}
