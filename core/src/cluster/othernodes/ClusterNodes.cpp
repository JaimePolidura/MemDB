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

std::vector<node_t> ClusterNodes::getAllNodes() {
    std::vector<node_t> nodes{};
    nodes.reserve(this->nodesById.size());
    for (const auto& entry : this->nodesById) {
        nodes.push_back(entry.second);
    }

    return nodes;
}

int ClusterNodes::getSize() {
    return this->nodesById.size();
}

void ClusterNodes::setNumberPartitions(uint32_t numberPartitions) {
    for (int i = 0; i < numberPartitions; ++i) {
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

std::result<Response> ClusterNodes::sendRequestToAnyNode(const Request& request, bool requiresSuccessfulResponse, const NodePartitionOptions options) {
    return this->sendRequestToAnyNode(requiresSuccessfulResponse, options, [&request](node_t node){return request;});
}

std::result<Response> ClusterNodes::sendRequestToAnyNode(bool requiresSuccessfulResponse,
                                             const NodePartitionOptions options,
                                             std::function<Request(node_t nodeToSend)> requestCreator) {
    std::set<memdbNodeId_t> alreadyCheckedNodeId{};

    while(true) {
        std::optional<node_t> node = this->getRandomNode(options, alreadyCheckedNodeId);

        if(!node.has_value()){
            return std::error<Response>();
        }

        Request request = requestCreator(node.value());

        std::result<Response> responseOptional = node.value()->sendRequest(request);

        alreadyCheckedNodeId.insert(node.value()->nodeId);

        if(responseOptional->isSuccessful && (!requiresSuccessfulResponse || responseOptional->isSuccessful)){
            return responseOptional;
        }
    }
}

auto ClusterNodes::sendRequest(memdbNodeId_t nodeId, const Request& request) -> Response {
    int timeout = this->configuration->get<int>(ConfigurationKeys::NODE_REQUEST_TIMEOUT_MS);
    node_t node = this->nodesById.at(nodeId);

    return Utils::retryUntilSuccessAndGet<Response, std::milli>(std::chrono::milliseconds(timeout), [node, &request]() -> std::result<Response> {
        return node->sendRequest(request);
    });
}

auto ClusterNodes::sendRequestToRandomNode(const Request& request, const NodePartitionOptions options) -> std::result<Response> {
    int timeout = this->configuration->get<int>(ConfigurationKeys::NODE_REQUEST_TIMEOUT_MS);
    int nRetries = this->configuration->get<int>(ConfigurationKeys::NODE_REQUEST_N_RETRIES);
    std::set<memdbNodeId_t> alreadyCheckedNodesId = {};

    return Utils::retryNTimesAndGet<Response, std::milli>(nRetries, std::chrono::milliseconds(timeout), [this, &request, &alreadyCheckedNodesId, options]() -> std::result<Response> {
        node_t nodeToSendRequest = Utils::getOptionalOrThrow<node_t>(this->getRandomNode(options, alreadyCheckedNodesId));
        
        return nodeToSendRequest->sendRequest(this->prepareRequest(request.operation));
    });
}

auto ClusterNodes::broadcast(const NodePartitionOptions options, const OperationBody& operation) -> void {
    std::set<memdbNodeId_t> allNodesIdInPartition = this->nodesInPartitions[options.partitionId].getAll();
    int timeout = this->configuration->get<int>(ConfigurationKeys::NODE_REQUEST_TIMEOUT_MS);
    int nRetries = this->configuration->get<int>(ConfigurationKeys::NODE_REQUEST_N_RETRIES);

    this->forEachNodeInPartition(options.partitionId, [this, timeout, nRetries, operation](node_t node) -> void {
        this->requestPool.submit([node, operation, timeout, nRetries, this]() mutable -> void {
            Utils::retryNTimesAndGet<Response, std::milli>(nRetries, std::chrono::milliseconds(timeout), [this, &node, &operation]() -> std::result<Response> {
                return node->sendRequest(this->prepareRequest(operation));
            });
        });
    });
}

auto ClusterNodes::broadcastAndWait(const NodePartitionOptions options, const OperationBody& operation) -> multipleResponses_t {
    int nNodesInPartition = this->nodesInPartitions[options.partitionId].size();
    multipleResponses_t multipleResponses = std::make_shared<MultipleResponses>(nNodesInPartition);
    MultipleResponsesNotifier multipleResponseNotifier(multipleResponses);

    this->forEachNodeInPartition(options.partitionId, [this, multipleResponseNotifier, operation](node_t node) mutable -> void {
        this->requestPool.submit([node, operation, multipleResponseNotifier, this]() mutable -> void {
            std::result<Response> responseResult = node->sendRequest(this->prepareRequest(operation));
            if (responseResult->isSuccessful) {
                multipleResponseNotifier.addResponse(node->nodeId, responseResult.get());
            }
        });
    });

    return multipleResponses;
}

void ClusterNodes::broadcastAll(const OperationBody& operation) {
    int timeout = this->configuration->get<int>(ConfigurationKeys::NODE_REQUEST_TIMEOUT_MS);
    int nRetries = this->configuration->get<int>(ConfigurationKeys::NODE_REQUEST_N_RETRIES);

    for (node_t node: this->nodesById | std::views::values) {
        this->requestPool.submit([node, operation, timeout, nRetries, this]() mutable -> void {
            Utils::retryNTimesAndGet<Response, std::milli>(nRetries, std::chrono::milliseconds(timeout), [this, &node, &operation]() -> std::result<Response> {
                return node->sendRequest(this->prepareRequest(operation));
            });
        });
    }
}

std::optional<node_t> ClusterNodes::getRandomNode(const NodePartitionOptions options, std::set<memdbNodeId_t> alreadyCheckedNodesId) {
    std::srand(std::time(nullptr));
    NodesInPartition nodesInPartition = this->nodesInPartitions[options.partitionId];
    std::set<memdbNodeId_t> nodesIdInPartition = nodesInPartition.getAll();

    while(alreadyCheckedNodesId.size() != nodesInPartition.size()) {
        memdbNodeId_t offset = std::rand() % nodesInPartition.size();

        if(alreadyCheckedNodesId.contains(offset))
            continue;

        auto ptr = std::begin(nodesIdInPartition);
        std::advance(ptr, offset);
        node_t randomNode = this->nodesById.at(* ptr);

        alreadyCheckedNodesId.insert(randomNode->nodeId);
    }

    return std::nullopt;
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

void ClusterNodes::forEachNodeInPartition(int partitionId, std::function<void(node_t node)> consumer) {
    std::set<memdbNodeId_t> allNodesIdInPartition = this->nodesInPartitions[partitionId].getAll();

    for(memdbNodeId_t nodeId : allNodesIdInPartition){
        consumer(this->nodesById.at(nodeId));
    }
}