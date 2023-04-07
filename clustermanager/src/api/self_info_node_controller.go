package api

import (
	"clustermanager/src/_shared/logging"
	"clustermanager/src/_shared/nodes"
	"github.com/labstack/echo/v4"
	"net"
	"net/http"
	"strconv"
)

type SelfNodeInfoController struct {
	NodesRepository nodes.NodeRepository
	Logger          *logging.Logger
}

type SelfNodeInfoResponse struct {
	OtherNodes []nodes.Node `json:"otherNodes"`
	Self       nodes.Node   `json:"self"`
}

func (controller *SelfNodeInfoController) GetSelfInfoNode(context echo.Context) error {
	ip, err := controller.getIpRequest(context)

	if err != nil {
		return err
	}

	nodeByIp, err := controller.NodesRepository.FindByAddress(ip)
	allNodes, err := controller.NodesRepository.FindAll()

	if err != nil {
		return context.JSON(http.StatusNotFound, "Node not found your ip "+ip)
	}

	controller.Logger.Log("Executed GetSelfInfoNode for node " + strconv.Itoa(int(nodeByIp.NodeId)))

	return context.JSON(http.StatusOK, SelfNodeInfoResponse{
		OtherNodes: controller.filterNodesExceptSelf(nodeByIp.NodeId, allNodes),
		Self:       nodeByIp,
	})
}

func (controller *SelfNodeInfoController) filterNodesExceptSelf(selfNodeId nodes.NodeId_t, allNodes []nodes.Node) []nodes.Node {
	nodesToReturn := make([]nodes.Node, len(allNodes)-1)

	for index, node := range allNodes {
		if node.NodeId == selfNodeId {
			return append(nodesToReturn, allNodes[index+1:]...)
		}

		nodesToReturn[index] = node
	}

	return nodesToReturn
}

func (controller *SelfNodeInfoController) getIpRequest(context echo.Context) (string, error) {
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
