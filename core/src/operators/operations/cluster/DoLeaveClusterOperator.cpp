#include "DoLeaveClusterOperator.h"

Response DoLeaveClusterOperator::operate(const OperationBody& operation, const OperationOptions operationOptions, OperatorDependencies& dependencies) {
    dependencies.logger->info("Received DO_LEAVE_CLUSTER. Leaving cluster in 5s");

    std::thread exitSelfThread = std::thread{[dependencies]() -> void {
        std::this_thread::sleep_for(std::chrono::seconds(5));

        dependencies.cluster->announceLeave();
        dependencies.cluster->deletedNodeInClusterHandler(std::make_shared<Node>(dependencies.cluster->getNodeId(),
            dependencies.configuration->get(ConfigurationKeys::ADDRESS), 0));
    }};

    return Response::success();
}

OperatorDescriptor DoLeaveClusterOperator::desc() {
    return OperatorDescriptor {
        .type = OperatorType::NODE_MAINTENANCE,
        .number = OperatorNumbers::DO_LEAVE_CLUSTER,
        .name = "DO_LEAVE_CLUSTER",
        .authorizedToExecute = { AuthenticationType::USER }
    };
}
