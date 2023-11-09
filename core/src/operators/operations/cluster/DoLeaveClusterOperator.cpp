#include "DoLeaveClusterOperator.h"

Response DoLeaveClusterOperator::operate(const OperationBody& operation, const OperationOptions operationOptions, OperatorDependencies& dependencies) {
    std::thread exitSelfThread = std::thread{[dependencies]() -> void {
        std::this_thread::sleep_for(std::chrono::seconds(5));

        dependencies.cluster->announceLeave();
        dependencies.cluster->clusterChangeHandler->handleDeletionNode(std::make_shared<Node>(dependencies.cluster->getNodeId(),
            dependencies.configuration->get(ConfigurationKeys::ADDRESS), NodeState::BOOTING, 0));
    }};

    return Response::success();
}

OperatorDescriptor DoLeaveClusterOperator::desc() {
    return OperatorDescriptor {
        .type = OperatorType::NODE_MAINTENANCE,
        .number = OperatorNumbers::DO_LEAVE_CLUSTER,
        .name = "DO_LEAVE_CLUSTER",
        .authorizedToExecute = { AuthenticationType::CLUSTER_MANAGER }
    };
}
