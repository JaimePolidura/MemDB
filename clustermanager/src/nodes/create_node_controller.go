package nodes

import (
	configuration "clustermanager/src/_shared/config"
	configuration_keys "clustermanager/src/_shared/config/keys"
	"clustermanager/src/nodes/nodes"
	"clustermanager/src/partitions"
	"errors"
	"fmt"
	"github.com/labstack/echo/v4"
	"net/http"
)

type CreateNodeController struct {
	NodesRepository   nodes.NodeRepository
	RingNodeAllocator *partitions.RingNodeAllocator
	Configuration     *configuration.Configuartion
}

type CreateNodeRequest struct {
	NodeId  nodes.NodeId_t `json:"nodeId"`
	Address string         `json:"address"`
}

func (controller *CreateNodeController) CreateNode(context echo.Context) error {
	request := new(CreateNodeRequest)
	if err := context.Bind(request); err != nil {
		return context.JSON(http.StatusBadRequest, "Invalid request body")
	}

	err := controller.ensureNodeIdNotTaken(request)
	err = controller.ensureNodeAddressNotTaken(request)

	if err != nil {
		return context.JSON(http.StatusBadRequest, err)
	}

	//TODO Add transactions to etcdclient
	if controller.Configuration.GetBoolean(configuration_keys.MEMDB_CLUSTERMANAGER_USE_PARTITIONS) {
		if !controller.RingNodeAllocator.CanAllocateNode(request.NodeId) {
			return context.JSON(http.StatusBadRequest, "All neighbor nodes have to be in RUNNING state")
		}

		_, err = controller.RingNodeAllocator.Allocate(request.NodeId)
	}
	err = controller.NodesRepository.Add(nodes.Node{
		NodeId:  request.NodeId,
		Address: request.Address,
		State:   nodes.BOOTING,
	})

	if err != nil {
		return context.JSON(http.StatusInternalServerError, err)
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
