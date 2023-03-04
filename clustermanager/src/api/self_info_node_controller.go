package api

import (
	"clustermanager/src/_shared/nodes"
	"github.com/labstack/echo/v4"
	"net"
	"net/http"
)

type SetupNodeController struct {
	NodesRepository nodes.NodeRepository
}

type SetupNodeResponse struct {
	OtherNodes []nodes.Node `json:"otherNodes"`
	Self       nodes.Node   `json:"self"`
}

func (controller *SetupNodeController) SetupNode(context echo.Context) error {
	ip, err := controller.getIpRequest(context)

	if err != nil {
		return err
	}

	nodeByIp, err := controller.NodesRepository.FindByAddress(ip)
	allNodes, err := controller.NodesRepository.FindAll()
	
	if err != nil {
		return context.JSON(http.StatusNotFound, "Node not found your ip "+ip)
	}

	return context.JSON(http.StatusOK, SetupNodeResponse{
		OtherNodes: controller.filterNodesExceptSelf(nodeByIp.NodeId, allNodes),
		Self:       nodeByIp,
	})
}

func (controller *SetupNodeController) filterNodesExceptSelf(selfNodeId nodes.NodeId_t, allNodes []nodes.Node) []nodes.Node {
	nodesToReturn := make([]nodes.Node, len(allNodes)-1)

	for index, node := range allNodes {
		if node.NodeId == selfNodeId {
			return append(nodesToReturn, allNodes[index+1:]...)
		}

		nodesToReturn[index] = node
	}

	return nodesToReturn
}

func (controller *SetupNodeController) getIpRequest(context echo.Context) (string, error) {
	ip, _, err := net.SplitHostPort(context.Request().RemoteAddr)

	if err != nil {
		return "", err
	}

	if ip == "::1" {
		return "127.0.0.1", nil
	} else {
		return ip, nil
	}
}
