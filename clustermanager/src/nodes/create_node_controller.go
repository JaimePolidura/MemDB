package nodes

import (
	"clustermanager/src/nodes/shared"
	"clustermanager/src/nodes/shared/states"
	"errors"
	"fmt"
	"github.com/labstack/echo/v4"
	"net/http"
)

type CreateNodeController struct {
	NodesRepository shared.NodeRepository
}

type CreateNodeRequest struct {
	NodeId  shared.NodeId_t `json:"nodeId"`
	Address string          `json:"address"`
}

func (controller *CreateNodeController) CreateNode(context echo.Context) error {
	request := new(CreateNodeRequest)
	if err := context.Bind(request); err != nil {
		return context.JSON(http.StatusBadRequest, "Invalid request body")
	}

	err := controller.ensureNodeIdNotTaken(request)
	err = controller.ensureNodeAddressNotTaken(request)

	if err != nil {
		context.JSON(http.StatusBadRequest, err.Error())
	}

	err = controller.NodesRepository.Add(shared.Node{
		NodeId:  request.NodeId,
		Address: request.Address,
		State:   states.BOOTING,
	})

	if err != nil {
		return context.JSON(http.StatusInternalServerError, err.Error())
	}

	return context.JSON(http.StatusCreated, "Node created")
}

func (controller *CreateNodeController) ensureNodeAddressNotTaken(request *CreateNodeRequest) error {
	_, err := controller.NodesRepository.FindByAddress(request.Address)

	if err == nil {
		return errors.New("Node with address " + request.Address + " already taken")
	} else {
		return nil
	}
}

func (controller *CreateNodeController) ensureNodeIdNotTaken(request *CreateNodeRequest) error {
	_, err := controller.NodesRepository.FindById(request.NodeId)

	if err == nil {
		return errors.New("Node with nodeId " + fmt.Sprint(request.NodeId) + " already taken")
	} else {
		return nil
	}
}
