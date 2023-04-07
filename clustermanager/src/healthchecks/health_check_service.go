package healthchecks

import (
	"clustermanager/src/_shared/config"
	"clustermanager/src/_shared/config/keys"
	"clustermanager/src/_shared/logging"
	"clustermanager/src/_shared/nodes"
	"clustermanager/src/_shared/nodes/connection"
	"clustermanager/src/_shared/nodes/connection/messages/request"
	"clustermanager/src/_shared/nodes/connection/messages/response"
	"clustermanager/src/_shared/nodes/states"
	"fmt"
	"strconv"
	"sync"
	"time"
)

type HealthCheckService struct {
	NodesRespository nodes.NodeRepository
	Configuration    *configuration.Configuartion
	NodeConnections  *connection.NodeConnections
	Logger           *logging.Logger

	periodHealthCheck       int64 //both expressed in seconds
	sendingHealthChecksLock sync.Mutex
}

func (healthCheckService *HealthCheckService) Start() {
	healthCheckService.periodHealthCheck = healthCheckService.Configuration.GetInt(configuration_keys.MEMDB_CLUSTERMANAGER_HEALTHCHECK_PERIOD)

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
	healthCheckService.Logger.Log("Starting health check round")
	nodesFromRepository, err := healthCheckService.NodesRespository.FindAll()

	if err != nil {
		_ = fmt.Errorf("[ClusterManager] Error while retrieving all nodes from database: %v\n", err)
		return
	}

	var waitGroup sync.WaitGroup

	for _, node := range nodesFromRepository {
		if node.State == states.BOOTING {
			continue
		}

		go healthCheckService.sendHealthCheckToNode(node, &waitGroup)
	}

	waitGroup.Wait()
}

func (healthCheckService *HealthCheckService) sendHealthCheckToNode(node nodes.Node, waitGroup *sync.WaitGroup) {
	waitGroup.Add(1)

	connectionToNode, err := healthCheckService.NodeConnections.GetByIdOrCreate(node)

	if err != nil {
		healthCheckService.NodesRespository.Add(*node.WithState(states.BOOTING))
		waitGroup.Done()
		return
	}

	authKey := healthCheckService.Configuration.Get(configuration_keys.MEMDB_CLUSTERMANAGER_AUTH_CLUSTER_KEY)
	response, err := connectionToNode.Send(request.BuildHealthCheckRequest(authKey))

	healthCheckService.logHealthCheckResult(response, node)

	if err != nil || !response.Success {
		healthCheckService.NodesRespository.Add(*node.WithState(states.SHUTDOWN))
		healthCheckService.NodeConnections.Delete(node.NodeId)
	} else if node.State == states.SHUTDOWN && response.Success {
		healthCheckService.NodesRespository.Add(*node.WithState(states.BOOTING))
		healthCheckService.NodeConnections.Create(node)
	}

	waitGroup.Done()
}

func (healthCheckService *HealthCheckService) logHealthCheckResult(response response.Response, node nodes.Node) {
	if response.Success {
		healthCheckService.Logger.Log("Recieved successful health check to " + strconv.Itoa(int(node.NodeId)))
	} else {
		healthCheckService.Logger.Log("Recieved failure health check to " + strconv.Itoa(int(node.NodeId)))
	}

}
