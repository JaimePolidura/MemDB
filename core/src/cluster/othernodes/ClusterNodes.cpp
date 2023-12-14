#include "cluster/othernodes/ClusterNodes.h"

void ClusterNodes::addNodesInPartition(const std::vector<node_t>& otherNodesToSet, memdbPartitionId_t partitionId) {
    for (const node_t& node: otherNodesToSet) {
        if(this->allNodesById.contains(node->nodeId)) {
            this->allNodesById[node->nodeId]->closeConnection();
            this->allNodesById.erase(node->nodeId);
        }

        this->allNodesById[node->nodeId] = node;

        NodesInPartition& nodesInPartition = this->nodesInPartitions[partitionId];
        nodesInPartition.add(node->nodeId);
    }
}

std::vector<node_t> ClusterNodes::getAllNodes() {
    std::vector<node_t> nodes{};
    nodes.reserve(this->allNodesById.size());
    for (const node_t node: this->allNodesById | std::views::values) {
        nodes.push_back(node);
    }

    return nodes;
}

void ClusterNodes::sendHintedHandoff(memdbNodeId_t nodeId) {
    iterator_t<Request> hints = this->hintsService->iterator(nodeId);
    while(hints->hasNext()) {
        this->sendRequest(nodeId, hints->next());
    }
}

void ClusterNodes::addNodes(const std::vector<node_t>& nodes) {
    for (node_t node : nodes) {
        this->addNode(node);
    }
}

int ClusterNodes::getSize() {
    return this->allNodesById.size();
}

void ClusterNodes::setNumberPartitions(uint32_t numberPartitions) {
    for (int i = 0; i < numberPartitions; ++i) {
        this->nodesInPartitions.push_back(NodesInPartition{});
    }
}

void ClusterNodes::removeNodeFromPartition(memdbNodeId_t nodeId, memdbPartitionId_t partitionId) {
    NodesInPartition& nodesInPartition = this->nodesInPartitions.at(partitionId);
    nodesInPartition.remove(nodeId);
}

std::vector<NodesInPartition> ClusterNodes::getNodesInPartitions() {
    return this->nodesInPartitions;
}

bool ClusterNodes::isEmtpy(memdbPartitionId_t partitionId) {
    return partitionId >= this->nodesInPartitions.size() || this->nodesInPartitions[partitionId].size() == 0;
}

void ClusterNodes::addNode(node_t node) {
    this->allNodesById[node->nodeId] = node;
}

void ClusterNodes::addNodeInPartition(node_t node, memdbPartitionId_t partitionId) {
    NodesInPartition& nodePartitions = this->nodesInPartitions[partitionId];
    nodePartitions.add(node->nodeId);
    this->allNodesById[node->nodeId] = node;
}

bool ClusterNodes::existsByNodeId(memdbNodeId_t nodeId) {
    return this->allNodesById.count(nodeId) != 0;
}

std::optional<node_t> ClusterNodes::getByNodeId(memdbNodeId_t nodeId) {
    if(this->allNodesById.contains(nodeId)) {
        return this->allNodesById[nodeId];
    } else {
        return std::nullopt;
    }
}

void ClusterNodes::deleteNodeById(const memdbNodeId_t nodeId) {
    this->allNodesById.erase(nodeId);

    for(NodesInPartition& nodesInPartition : this->nodesInPartitions){
        nodesInPartition.remove(nodeId);
    }
}

std::result<Response> ClusterNodes::sendRequestToAnyNode(const Request& request, bool requiresSuccessfulResponse, const SendRequestOptions options) {
    return this->sendRequestToAnyNode(requiresSuccessfulResponse, options, [&request](node_t node){return request;});
}

std::result<Response> ClusterNodes::sendRequestToAnyNode(bool requiresSuccessfulResponse,
                                             const SendRequestOptions options,
                                             std::function<Request(node_t nodeToSend)> requestCreator) {
    std::set<memdbNodeId_t> alreadyCheckedNodeId{};

    while(true) {
        std::optional<node_t> node = this->getRandomNode(options.partitionId, alreadyCheckedNodeId);

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
    node_t node = this->allNodesById.at(nodeId);

    return Utils::retryUntilSuccessAndGet<Response, std::milli>(std::chrono::milliseconds(timeout), [node, &request]() -> std::result<Response> {
        return node->sendRequest(request);
    });
}

auto ClusterNodes::sendRequestToRandomNode(const Request& request, const SendRequestOptions options) -> std::result<Response> {
    int timeout = this->configuration->get<int>(ConfigurationKeys::NODE_REQUEST_TIMEOUT_MS);
    int nRetries = this->configuration->get<int>(ConfigurationKeys::NODE_REQUEST_N_RETRIES);
    std::set<memdbNodeId_t> alreadyCheckedNodesId = {};

    return Utils::retryNTimesAndGet<Response, std::milli>(nRetries, std::chrono::milliseconds(timeout), [this, &request, &alreadyCheckedNodesId, options]() -> std::result<Response> {
        node_t nodeToSendRequest = Utils::getOptionalOrThrow<node_t>(this->getRandomNode(options.partitionId, alreadyCheckedNodesId));
        
        return nodeToSendRequest->sendRequest(this->prepareRequest(request.operation));
    });
}

auto ClusterNodes::broadcast(const OperationBody& operation, SendRequestOptions options) -> void {
    std::set<memdbNodeId_t> allNodesIdInPartition = this->nodesInPartitions[options.partitionId].getAll();
    int timeout = this->configuration->get<int>(ConfigurationKeys::NODE_REQUEST_TIMEOUT_MS);
    int nRetries = this->configuration->get<int>(ConfigurationKeys::NODE_REQUEST_N_RETRIES);

    this->forEachNodeInPartition(options.partitionId, [this, timeout, nRetries, options, operation](node_t node) -> void {
        this->requestPool.submit([node, operation, timeout, nRetries, options, this]() mutable -> void {
            std::result<Response> result = Utils::retryNTimesAndGet<Response, std::milli>(nRetries, std::chrono::milliseconds(timeout),
                [this, &node, &operation]() -> std::result<Response> {
                    return node->sendRequest(this->prepareRequest(operation));
            });

            if(result.has_error() && options.canBeStoredInHint) {
                this->hintsService->add(node->nodeId, this->prepareRequest(operation));
            }
        });
    });
}

auto ClusterNodes::broadcastAndWait(const OperationBody& operation, SendRequestOptions options) -> multipleResponses_t {
    int nNodesInPartition = this->nodesInPartitions[options.partitionId].size();
    multipleResponses_t multipleResponses = std::make_shared<MultipleResponses>(nNodesInPartition);
    MultipleResponsesNotifier multipleResponseNotifier(multipleResponses);

    this->forEachNodeInPartition(options.partitionId, [this, options, multipleResponseNotifier, operation](node_t node) mutable -> void {
        this->requestPool.submit([node, options, operation, multipleResponseNotifier, this]() mutable -> void {
            std::result<Response> responseResult = node->sendRequest(this->prepareRequest(operation));
            if (responseResult->isSuccessful) {
                multipleResponseNotifier.addResponse(node->nodeId, responseResult.get());
            }
            if(responseResult.has_error() && options.canBeStoredInHint) {
                this->hintsService->add(node->nodeId, this->prepareRequest(operation));
            }
        });
    });

    return multipleResponses;
}

void ClusterNodes::broadcastAll(const OperationBody& operation, SendRequestOptions options) {
    int timeout = this->configuration->get<int>(ConfigurationKeys::NODE_REQUEST_TIMEOUT_MS);
    int nRetries = this->configuration->get<int>(ConfigurationKeys::NODE_REQUEST_N_RETRIES);

    for (node_t node: this->allNodesById | std::views::values) {
        this->requestPool.submit([node, operation, timeout, options, nRetries, this]() mutable -> void {
            std::result<Response> result = Utils::retryNTimesAndGet<Response, std::milli>(nRetries, std::chrono::milliseconds(timeout),
                [this, &node, &operation, options]() -> std::result<Response> {
                    return node->sendRequest(this->prepareRequest(operation));
            });

            if(result.has_error() && options.canBeStoredInHint) {
               this->hintsService->add(node->nodeId, this->prepareRequest(operation));
            }
        });
    }
}

std::optional<node_t> ClusterNodes::getRandomNode(memdbPartitionId_t partitionId, std::set<memdbNodeId_t>& alreadyCheckedNodesId) {
    std::srand(std::time(nullptr));
    NodesInPartition nodesInPartition = this->nodesInPartitions[partitionId];
    std::set<memdbNodeId_t> nodesIdInPartition = nodesInPartition.getAll();

    while(alreadyCheckedNodesId.size() != nodesInPartition.size()) {
        memdbNodeId_t offset = std::rand() % nodesInPartition.size();

        if(alreadyCheckedNodesId.contains(offset))
            continue;

        auto ptr = std::begin(nodesIdInPartition);
        std::advance(ptr, offset);
        node_t randomNode = this->allNodesById.at(* ptr);

        alreadyCheckedNodesId.insert(randomNode->nodeId);

        return randomNode;
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
        consumer(this->allNodesById.at(nodeId));
    }
}