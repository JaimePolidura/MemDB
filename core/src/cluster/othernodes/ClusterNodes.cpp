#include "cluster/othernodes/ClusterNodes.h"

void ClusterNodes::setOtherNodes(const std::vector<node_t>& otherNodesToSet, const NodePartitionOptions options) {
    for (const node_t& node: otherNodesToSet) {
        if(this->nodesById.contains(node->nodeId)) {
            this->nodesById[node->nodeId]->closeConnection();
            this->nodesById.erase(node->nodeId);
        }

        this->nodesById[node->nodeId] = node;

        NodesInPartition& nodesInPartition = this->nodesInPartitions[options.partitionId];
        nodesInPartition.add(node->nodeId);
    }
}

void ClusterNodes::setNumberPartitions(uint32_t numberPartitions) {
    for (int i = 0; i < numberPartitions; ++i) {
        this->nodesById.size();

        this->nodesInPartitions.push_back(NodesInPartition{});
    }
}

void ClusterNodes::removeNodeFromPartition(memdbNodeId_t nodeId, const NodePartitionOptions options) {
    NodesInPartition& nodesInPartition = this->nodesInPartitions.at(options.partitionId);
    nodesInPartition.remove(nodeId);
}

std::vector<NodesInPartition> ClusterNodes::getNodesInPartitions() {
    return this->nodesInPartitions;
}

bool ClusterNodes::isEmtpy(const NodePartitionOptions options) {
    return options.partitionId >= this->nodesInPartitions.size() || this->nodesInPartitions[options.partitionId].size() == 0;
}

void ClusterNodes::setNodeState(memdbNodeId_t nodeId, const NodeState newState) {
    this->nodesById[nodeId]->state = newState;
}

void ClusterNodes::addNode(node_t node, const NodePartitionOptions options) {
    NodesInPartition& nodePartitions = this->nodesInPartitions[options.partitionId];
    nodePartitions.add(node->nodeId);
    this->nodesById[node->nodeId] = node;
}

bool ClusterNodes::existsByNodeId(memdbNodeId_t nodeId) {
    return this->nodesById.count(nodeId) != 0;
}

node_t ClusterNodes::getByNodeId(memdbNodeId_t nodeId) {
    return this->nodesById[nodeId];
}

void ClusterNodes::deleteNodeById(const memdbNodeId_t nodeId) {
    this->nodesById.erase(nodeId);

    for(NodesInPartition& nodesInPartition : this->nodesInPartitions){
        nodesInPartition.remove(nodeId);
    }
}

auto ClusterNodes::sendRequest(memdbNodeId_t nodeId, const Request& request) -> Response {
    int timeout = this->configuration->get<int>(ConfigurationKeys::NODE_REQUEST_TIMEOUT_MS);
    node_t node = this->nodesById.at(nodeId);

    return Utils::retryUntilSuccessAndGet<Response, std::milli>(std::chrono::milliseconds(timeout), [node, &request]() -> Response {
        return node->sendRequest(request, true).value();
    });
}

auto ClusterNodes::sendRequestToRandomNode(const Request& request, const NodePartitionOptions options) -> std::optional<Response> {
    int timeout = this->configuration->get<int>(ConfigurationKeys::NODE_REQUEST_TIMEOUT_MS);
    int nRetries = this->configuration->get<int>(ConfigurationKeys::NODE_REQUEST_N_RETRIES);
    std::set<memdbNodeId_t> alreadyCheckedNodesId = {};

    return Utils::retryUntilAndGet<Response, std::milli>(nRetries, std::chrono::milliseconds(timeout), [this, &request, &alreadyCheckedNodesId, options]() -> Response {
        node_t nodeToSendRequest = this->getRandomNode(alreadyCheckedNodesId, options);

        return nodeToSendRequest->sendRequest(this->prepareRequest(request.operation), true).value();
    });
}

auto ClusterNodes::broadcast(const OperationBody& operation, const NodePartitionOptions options) -> void {
    std::set<memdbNodeId_t> allNodesIdInPartition = this->nodesInPartitions[options.partitionId].getAll();
    int timeout = this->configuration->get<int>(ConfigurationKeys::NODE_REQUEST_TIMEOUT_MS);
    int nRetries = this->configuration->get<int>(ConfigurationKeys::NODE_REQUEST_N_RETRIES);

    for(memdbNodeId_t nodeId : allNodesIdInPartition){
        node_t node = this->nodesById.at(nodeId);

        if(!NodeStates::canAcceptRequest(node->state)) {
            continue;
        }

        this->requestPool.submit([node, operation, timeout, nRetries, this]() mutable -> void {
            Utils::retryUntil(nRetries, std::chrono::milliseconds(timeout), [this, &node, &operation]() -> void{
                node->sendRequest(this->prepareRequest(operation), false);
            });
        });
    }
}

node_t ClusterNodes::getRandomNode(std::set<memdbNodeId_t> alreadyCheckedNodesId, const NodePartitionOptions options) {
    std::srand(std::time(nullptr));
    NodesInPartition nodesInPartition = this->nodesInPartitions[options.partitionId];
    std::set<memdbNodeId_t> nodesIdInPartition = nodesInPartition.getAll();

    std::set<memdbNodeId_t> alreadyChecked{};

    while(alreadyCheckedNodesId.size() != nodesInPartition.size()) {
        memdbNodeId_t offset = std::rand() % nodesInPartition.size();

        if(alreadyChecked.contains(offset))
            continue;

        auto ptr = std::begin(nodesIdInPartition);
        std::advance(ptr, offset);
        node_t randomNode = this->nodesById.at(* ptr);
        
        if(Node::canSendRequestUnicast(randomNode->state))
            return randomNode;

        alreadyCheckedNodesId.insert(randomNode->nodeId);
    }

    throw std::runtime_error("No node available for selecting");
}

Request ClusterNodes::prepareRequest(const OperationBody& operation) {
    Request request{};

    AuthenticationBody authenticationBody{};
    authenticationBody.authKey = this->configuration->get(ConfigurationKeys::AUTH_NODE_KEY);
    authenticationBody.flag1 = true;

    const_cast<OperationBody&>(operation).nodeId = this->configuration->get<memdbNodeId_t>(ConfigurationKeys::NODE_ID);

    request.operation = operation;
    request.authentication = authenticationBody;

    return request;
}