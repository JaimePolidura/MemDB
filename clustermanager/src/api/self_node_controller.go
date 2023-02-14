package self

import (
	"clustermanager/src/_shared/nodes"
	"github.com/labstack/echo/v4"
	"net"
	"net/http"
)

type SelfNodeController struct {
	NodesRepository nodes.NodeRepository
}

func (selfNodeController *SelfNodeController) GetSelfNode(context echo.Context) error {
	ip, err := selfNodeController.getIpRequest(context)

	if err != nil {
		return err
	}

	nodeByIp, err := selfNodeController.NodesRepository.FindByAddress(ip)

	if err != nil {
		return context.JSON(http.StatusNotFound, "Node not found for "+ip)
	}

	return context.JSON(http.StatusOK, nodeByIp)
}

func (selfNodeController *SelfNodeController) getIpRequest(context echo.Context) (string, error) {
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
