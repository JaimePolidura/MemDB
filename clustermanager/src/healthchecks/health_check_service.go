package healthchecks

import (
	configuration "clustermanager/src/config"
	configuration_keys "clustermanager/src/config/keys"
	"clustermanager/src/nodes"
	"clustermanager/src/nodes/states"
	"fmt"
	"sync"
	"time"
)

type HealthCheckService struct {
	NodesRespository nodes.NodeRepository
	Configuration    *configuration.Configuartion

	periodHealthCheck         int64 //both expressed in seconds
	healthCheckLastTimeRunned int64
}

func (healthCheckService *HealthCheckService) Start() {
	healthCheckService.periodHealthCheck = healthCheckService.Configuration.GetOrDefaultInt(configuration_keys.MEMDB_CLUSTERMANAGER_HEALTHCHECK_PERIOD, 60)

	go healthCheckService.startAsyncHealthCheckPeriodicRoutine()
}

func (healthCheckService *HealthCheckService) startAsyncHealthCheckPeriodicRoutine() {
	ticker := time.NewTicker(time.Duration(healthCheckService.periodHealthCheck) * time.Second)
	defer ticker.Stop()

	for {
		select {
		case <-ticker.C:
			timeElapsedSinceLastHealtCheck := time.Now().Unix() - healthCheckService.healthCheckLastTimeRunned

			if timeElapsedSinceLastHealtCheck >= healthCheckService.periodHealthCheck {
				healthCheckService.runHealthChecks()
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

	var waitGroup sync.WaitGroup

	for _, node := range nodesFromRepository {
		if node.State != states.RUNNING && node.State != states.CRITICAL {
			continue
		}

		go healthCheckService.sendHealthCheckToNode(node, &waitGroup)
	}

	waitGroup.Wait()
	healthCheckService.healthCheckLastTimeRunned = time.Now().Unix()
}

func (healthCheckService *HealthCheckService) sendHealthCheckToNode(node nodes.Node, waitGroup *sync.WaitGroup) {
	waitGroup.Add(1)

	//SEND TCP Request with cluster_key

	waitGroup.Done()
}
