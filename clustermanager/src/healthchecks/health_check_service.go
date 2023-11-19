package healthchecks

import (
	"clustermanager/src/config"
	"clustermanager/src/config/keys"
	"clustermanager/src/logging"
	"clustermanager/src/nodes/nodes"
	connection2 "clustermanager/src/nodes/nodes/connection"
	"clustermanager/src/nodes/nodes/connection/messages/request"
	"fmt"
	"sync"
	"time"
)

type HealthCheckService struct {
	NodesRespository *nodes.NodeRepository
	Configuration    *configuration.configuration
	NodeConnections  *connection2.NodeConnections
	Logger           *logging.Logger

	periodHealthCheck       int64 //both expressed in seconds
	sendingHealthChecksLock sync.Mutex
}

func (healthCheckService *HealthCheckService) Start() {
	healthCheckService.periodHealthCheck = healthCheckService.Configuration.GetInt(configuration_keys.configuration_keys.MEMDB_CLUSTERMANAGER_HEALTHCHECK_PERIOD)

	go healthCheckService.startAsyncHealthCheckPeriodicRoutine()
}

func (healthCheckService *HealthCheckService) startAsyncHealthCheckPeriodicRoutine() {
	ticker := time.NewTicker(time.Duration(healthCheckService.periodHealthCheck) * time.Second)
	defer ticker.Stop()

	for {
		select {
		case <-ticker.C:
			if healthCheckService.sendingHealthChecksLock.TryLock() {
				healthCheckService.runHealthChecks()
				healthCheckService.sendingHealthChecksLock.Unlock()
			}
		}
	}
}

func (healthCheckService *HealthCheckService) runHealthChecks() {
	nodesFromRepository, err := healthCheckService.NodesRespository.FindAll()

	if err != nil {
		_ = fmt.Errorf("[ClusterManager] Error while retrieving all nodes from database: %v\n", err)
		return
	}

	waitGroup := new(sync.WaitGroup)

	healthCheckService.Logger.Info("Starting healthcheck round")
	for _, node := range nodesFromRepository {
		go healthCheckService.sendHealthCheckToNode(node, waitGroup)
	}

	waitGroup.Wait()
}

func (healthCheckService *HealthCheckService) sendHealthCheckToNode(node nodes.Node, waitGroup *sync.WaitGroup) {
	waitGroup.Add(1)

	connectionToNode, err := healthCheckService.getConnectionOrCreate(node)

	if err != nil {
		if node.State != nodes.SHUTDOWN {
			healthCheckService.NodesRespository.Add(*node.WithState(nodes.SHUTDOWN))
		}
		waitGroup.Done()
		return
	}

	authKey := healthCheckService.Configuration.Get(configuration_keys.MEMDB_CLUSTERMANAGER_AUTH_MAINTENANCE_KEY)
	response, err := connectionToNode.Send(request.BuildHealthCheckRequest(authKey))
	didntRespondToHealthCheck := err != nil || !response.Success
	respondedToHealthCheck := !didntRespondToHealthCheck

	if didntRespondToHealthCheck && node.State != nodes.SHUTDOWN {
		healthCheckService.NodesRespository.Add(*node.WithState(nodes.SHUTDOWN))
		healthCheckService.NodeConnections.Delete(node.NodeId)
		healthCheckService.Logger.Info("Node" + string(node.NodeId) + " doest repond to health check. Marked as SHUTDOWN")
	} else if respondedToHealthCheck && node.State == nodes.SHUTDOWN {
		healthCheckService.NodesRespository.Add(*node.WithState(nodes.BOOTING))
		healthCheckService.NodeConnections.Create(node)
		healthCheckService.Logger.Info("Node" + string(node.NodeId) + " previously marked as SHUTDOWN, now it responds to health check. Marked as BOOTING")
	}

	waitGroup.Done()
}

func (healthCheckService *HealthCheckService) getConnectionOrCreate(node nodes.Node) (*connection2.NodeConnection, error) {
	if node.State == nodes.SHUTDOWN {
		healthCheckService.NodeConnections.Delete(node.NodeId)
	}

	return healthCheckService.NodeConnections.GetByIdOrCreate(node)
}
