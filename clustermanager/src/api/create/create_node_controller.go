package create

import (
	"clustermanager/src/_shared/nodes"
	"errors"
	"github.com/labstack/echo/v4"
)

type CreateNodeController struct {
	NodesRepository *nodes.NodeRepository
}

func (controller *CreateNodeController) createNode(contetext echo.Context) error {
	return errors.New("not implemented")
}
