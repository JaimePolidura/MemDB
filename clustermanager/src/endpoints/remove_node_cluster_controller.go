package endpoints

import (
	configuration "clustermanager/src/config"
	"clustermanager/src/nodes"
	"clustermanager/src/nodes/messages/request"
	"github.com/labstack/echo/v4"
	"net/http"
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
	authKey := this.Configuartion.Get(configuration.MEMDB_CLUSTERMANAGER_AUTH_USER_KEY)
	nodeId := nodes.NodeId_t(ctx.Param("nodeId"))

	node, err := this.ClusterNodeConnections.GetNode(nodeId)
	if err != nil {
		return ctx.JSON(http.StatusBadRequest, "Node not found / Impossible to contact with seeder node")
	}

	if res, err := node.Send(request.BuildDoLeaveNodeClusterRequest(authKey, node.NodeId)); err != nil || !res.Success {
		return ctx.JSON(http.StatusBadRequest, "Error while sending DO_LEAVE_CLUSTER")
	} else {
		return ctx.JSON(http.StatusOK, "Sended DO_LEAVE_CLUSTER")
	}
}
