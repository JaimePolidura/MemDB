#pragma once

#include "logging/Logger.h"
#include "cluster/Cluster.h"
#include "operators/OperatorDispatcher.h"
#include "persistence/OperationLogSerializer.h"
#include "persistence/OperationLogUtils.h"
#include "persistence/OperationLogInvalidator.h"

class DeletionNodeChangeHandler {
private:
    OperationLogInvalidator operationLogInvalidator;
    OperationLogSerializer operationLogSerializer;

    operatorDispatcher_t operatorDispatcher;
    operationLog_t operationLog;
    cluster_t cluster;
    logger_t logger;

public:
    DeletionNodeChangeHandler(logger_t logger, cluster_t cluster, operationLog_t operationLog, operatorDispatcher_t operatorDispatcher):
    logger(logger), cluster(cluster), operationLog(operationLog), operatorDispatcher(operatorDispatcher) {}

    DeletionNodeChangeHandler() = default;

    void handle(node_t deletedNode) {
        if(!this->cluster->partitions->isNeighbor(deletedNode->nodeId)){
            return;
        }

        RingEntry ringEntryDeleted = this->cluster->partitions->getRingEntryByNodeId(deletedNode->nodeId);
        cluster->setBooting();

        if(deletedNode->nodeId == this->cluster->selfNode->nodeId){
            //TODO XD
        }
    }
};