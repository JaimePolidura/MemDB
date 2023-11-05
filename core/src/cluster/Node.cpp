#include "cluster/Node.h"

Node::Node(): connection(nullptr) {}

Node::Node(memdbNodeId_t nodeId, const std::string& address, NodeState state, uint64_t readTimeout):
    connection(nullptr), nodeId(nodeId), address(address), state(state), readTimeout(readTimeout)
{}

Node::Node(const Node& other) {
    this->connection = other.connection;
    this->address = other.address;
    this->state = other.state;
    this->nodeId = other.nodeId;
    this->readTimeout = other.readTimeout;
    this->responseDeserializer = other.responseDeserializer;
    this->requestSerializer = other.requestSerializer;
}

auto Node::sendRequest(const Request &request) -> std::result<Response> {
    this->openConnectionIfClosedOrThrow();

    std::vector<uint8_t> serializedRequest = this->requestSerializer.serialize(request);
    std::size_t bytesWritten = this->writeSyncToConnection(serializedRequest);

    if(bytesWritten == 0){
        return std::error<Response>();
    }

    std::result<std::vector<uint8_t>> responseBytesResult = this->connection->readSync(this->readTimeout);
    if(responseBytesResult.has_error()){
        return std::error<Response>();
    }

    return this->responseDeserializer.deserialize(responseBytesResult.get());
}

void Node::closeConnection() {
    if(this->isConnectionOpened()){
        this->connection->close();
    }
}

bool Node::isConnectionOpened() const {
    return this->connection.get() != nullptr && this->connection->isOpen();
}

bool Node::openConnection() {
    if(!NodeStates::canAcceptRequest(this->state) || this->isConnectionOpened()){
        return false;
    }

    auto splitedAddress = StringUtils::split(this->address, ':');
    auto ip = splitedAddress[0];
    auto port = splitedAddress[1];

    boost::asio::ip::tcp::socket socket(this->ioContext);

    bool success = Utils::tryOnce([ip, port, &socket]() mutable -> void{
        if(DNSUtils::isName(ip))
            ip = DNSUtils::singleResolve(ip, port);

        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(ip), std::atoi(port.data()));
        socket.connect(endpoint);
    });

    if(!success)
        return false;

    this->connection = std::make_shared<Connection>(std::move(socket), this->logger);

    return true;
}

void Node::openConnectionIfClosedOrThrow() {
    if(!this->isConnectionOpened()){
        bool success = this->openConnection();

        if(!success){
            throw std::runtime_error("Cannot open connection");
        }
    }
}

void Node::setLogger(logger_t loggerP) {
    this->logger = loggerP;
}

bool Node::canSendRequestUnicast(NodeState state) {
    return state == NodeState::RUNNING;
}

std::size_t Node::writeSyncToConnection(std::vector<uint8_t>& bytes) {
    try {
        return this->connection->writeSync(bytes);
    }catch (const std::exception& e) {
        if(openConnection()){
            return Utils::tryOnceAndGet<std::size_t>([this, &bytes](){return this->connection->writeSync(bytes);}, 0);
        } else {
            return 0;
        }
    }
}

std::result<std::vector<uint8_t>> Node::readSyncFromConnection(uint64_t timeoutMs) {
    std::result<std::vector<uint8_t>> readResult = this->connection->readSync(timeoutMs);
    
    if(!readResult.is_success()){
        this->openConnection();
        readResult = this->connection->readSync(timeoutMs);
    }

    return readResult;
}

std::string Node::toJson(std::shared_ptr<Node> node) {
    return "{\"nodeId\": \""+std::to_string(node->nodeId)+"\", \"address\": \""+node->address+"\", \"state\": \""+NodeStates::parseNodeStateToString(node->state)+"\"}";
}

std::shared_ptr<Node> Node::fromJson(const nlohmann::json& json) {
    std::shared_ptr<Node> node = std::make_shared<Node>();
    node->address = json["address"].get<std::string>();
    node->state = NodeStates::parseNodeStateFromString(json["state"].get<std::string>());
    node->nodeId = (memdbNodeId_t) std::stoi(json["nodeId"].get<std::string>());

    return node;
}