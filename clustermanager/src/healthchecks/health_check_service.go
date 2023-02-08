package healthchecks

import (
	configuration "clustermanager/src/config"
	"clustermanager/src/nodes"
)

type HealthCheckService struct {
	NodesRespository nodes.NodeRepository
	Configuration    *configuration.Configuartion
}

func (start *HealthCheckService) Start() {
	
}
