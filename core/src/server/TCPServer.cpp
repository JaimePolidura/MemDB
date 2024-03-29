#include "server/TCPServer.h"

TCPServer::TCPServer(logger_t logger, configuration_t configuration, Authenticator authenticator, operatorDispatcher_t operatorDispatcher):
    configuration(configuration),
    logger(logger),
    port(configuration->get<uint16_t>(ConfigurationKeys::SERVER_PORT)),
    authenicator(std::move(authenticator)),
    connectionThreadPool(configuration->get<int>(ConfigurationKeys::SERVER_THREADS)),
    operatorDispatcher(operatorDispatcher),
    acceptator(ioContext, ip::tcp::endpoint{ip::tcp::v4(), this->port}) {};

void TCPServer::run() {
    this->logger->info("Listening for connections...");

    try{
        this->acceptNewConnections();

        this->ioContext.run();
    }catch (const std::exception& e) {
        this->logger->error("Fatal _error in TCP Server {0}", e.what());
    }
}

void TCPServer::acceptNewConnections() {
    this->acceptator.async_accept([this](std::error_code ec, ip::tcp::socket socket) {
        std::shared_ptr<Connection> connection = std::make_shared<Connection>(std::move(socket), this->logger);

        this->logger->debugInfo("Accepted TCP Connection {0}", connection->getAddress());

        connection->onRequest([connection, this](const std::vector<uint8_t>& requestRawBuffer) {
            this->connectionThreadPool.submit([connection, requestRawBuffer, this] {
                this->onNewRequest(requestRawBuffer, connection);
            });
        });

        connection->readAsync(this->configuration->get<uint64_t>(ConfigurationKeys::NODE_REQUEST_TIMEOUT_MS));

        this->acceptNewConnections();
    });
}

void TCPServer::onNewRequest(const std::vector<uint8_t>& requestRawBuffer, connection_t connection) {
    Request request = this->requestDeserializer.deserialize(requestRawBuffer);
    bool authenticationValid = this->authenicator.authenticate(request.authentication.authKey);

    if(!authenticationValid){
        this->sendAuthError(connection, request);
        return;
    }

    request.authenticationType = this->authenicator.getAuthenticationType(request.authentication.authKey).get();
    if(request.authenticationType != AuthenticationType::NODE && request.authentication.flag1){//Only node id can be present in request with node keys
        this->sendAuthError(connection, request);
        return;
    }

    Response response = this->operatorDispatcher->dispatch(request);
    response.requestNumber = request.requestNumber;

    this->sendResponse(connection, response);
}

void TCPServer::sendAuthError(connection_t connection, Request request) {
    const Response errorAuthResponse = Response::error(ErrorCode::AUTH_ERROR, request.requestNumber, LamportClock{0, request.operation.timestamp});
    this->sendResponse(connection, errorAuthResponse);
}

void TCPServer::sendResponse(std::shared_ptr<Connection> connection, const Response& response) {
    if(connection->isOpen()){
        std::vector<uint8_t> serialized = this->responseSerializer.serialize(response);
        connection->writeAsync(serialized);
    }
}