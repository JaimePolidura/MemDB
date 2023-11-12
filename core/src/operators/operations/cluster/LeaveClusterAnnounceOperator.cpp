#include "LeaveClusterAnnounceOperator.h"

Response LeaveClusterAnnounceOperator::operate(const OperationBody& operation, const OperationOptions operationOptions, OperatorDependencies&dependencies) {
    if(dependencies.cluster->clusterNodes->existsByNodeId(operation.nodeId)) {
        node_t node = dependencies.cluster->clusterNodes->getByNodeId(operation.nodeId);
        dependencies.cluster->deletedNodeInClusterHandler(node);
    }

    return Response::success();
}

OperatorDescriptor LeaveClusterAnnounceOperator::desc() {
    return OperatorDescriptor {
        .type = OperatorType::NODE_MAINTENANCE,
        .number = OperatorNumbers::LEAVE_CLUSTER_ANNOUNCE,
        .name = "LEAVE_CLUSTER_ANNOUNCE",
        .authorizedToExecute = { AuthenticationType::NODE }
    };
}
