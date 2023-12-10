package endpoints

import (
	configuration "clustermanager/src/config"
	"clustermanager/src/nodes"
	"clustermanager/src/nodes/messages"
	"github.com/labstack/echo/v4"
	"net/http"
	"strconv"
)

type RemoveNodeClusterController struct {
	ClusterNodeConnections *nodes.ClusterNodeConnections
	Configuartion          *configuration.Configuartion
}

func CreateRemoveNodeClusterController(configuartion *configuration.Configuartion, clusterNodeConnections *nodes.ClusterNodeConnections) *RemoveNodeClusterController {
	return &RemoveNodeClusterController{
		ClusterNodeConnections: clusterNodeConnections,
		Configuartion:          configuartion,
	}
}

func (this *RemoveNodeClusterController) Remove(ctx echo.Context) error {
	authKey := this.Configuartion.Get(configuration.AUTH_API_USER_KEY)
	nodeId := nodes.NodeId_t(ctx.Param("nodeId"))

	node, err := this.ClusterNodeConnections.GetNode(nodeId)
	if err != nil {
		return ctx.JSON(http.StatusBadRequest, "Node not found / Impossible to contact with seeder node")
	}
	
	if res, err := node.Send(messages.BuildDoLeaveNodeClusterRequest(authKey, string(node.NodeId))); err != nil {
		return ctx.JSON(http.StatusBadRequest, "Error while sending DO_LEAVE_CLUSTER: "+err.Error())
	} else if !res.Success {
		return ctx.JSON(http.StatusOK, "Error while sending DO_LEAVE_CLUSTER: "+strconv.Itoa(int(res.ErrorCode)))
	} else if res.Success {
		return ctx.JSON(http.StatusOK, "Sended DO_LEAVE_CLUSTER")
	}

	return nil
}
