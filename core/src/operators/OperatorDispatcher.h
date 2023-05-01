#pragma once

#include "operators/Operator.h"
#include "operators/OperatorRegistry.h"
#include "operators/DbOperatorExecutor.h"
#include "operators/MaintenanceOperatorExecutor.h"
#include "operators/dependencies/OperatorDependencies.h"
#include "operators/dependencies/OperatorDependency.h"

#include "messages/response/ErrorCode.h"
#include "persistence/oplog/SingleOperationLog.h"
#include "utils/clock/LamportClock.h"
#include "cluster/Cluster.h"
#include "cluster/PendingReplicationOperationBuffer.h"

class OperatorDispatcher {
public: //Need it for mocking it
    operatorRegistry_t operatorRegistry;
private:
    replicationOperationBuffer_t replicationOperationBuffer;
    operationLog_t operationLog;
    configuration_t configuration;
    cluster_t cluster;
    lamportClock_t clock;
    memDbDataStore_t db;
    logger_t logger;

public:
    OperatorDispatcher(memDbDataStore_t dbCons, lamportClock_t clock, cluster_t cluster, configuration_t configuration,
                       logger_t logger, operationLog_t operationLog): db(dbCons), operationLog(operationLog), clock(clock),
                       operatorRegistry(std::make_shared<OperatorRegistry>()), logger(logger), cluster(cluster),
                       configuration(configuration), replicationOperationBuffer(std::make_shared<PendingReplicationOperationBuffer>())
    {}

    Response dispatch(const Request& request) {
        this->applyReplicatedOperationBuffer();

        auto operatorToExecute = this->operatorRegistry->get(request.operation.operatorNumber);

        if(operatorToExecute.get() == nullptr){
            return Response::error(ErrorCode::UNKNOWN_OPERATOR);
        }
        if(!this->isAuthorizedToExecute(operatorToExecute, request.authenticationType)) {
            return Response::error(ErrorCode::NOT_AUTHORIZED);
        }

        OperationOptions options = {.requestOfNodeToReplicate = request.authenticationType == AuthenticationType::NODE &&
                                    operatorToExecute->type() == OperatorType::WRITE};

        this->logger->debugInfo("Recieved request for operator {0} from {1}", request.requestNumber, operatorToExecute->name(),
                                options.requestOfNodeToReplicate ? "node" : "user");

        if(this->isInReplicationMode() &&
            (!NodeStates::canAcceptRequest(this->cluster->getNodeState()) ||
            (!options.requestOfNodeToReplicate && !NodeStates::cantExecuteRequest(this->cluster->getNodeState())))) {
            return Response::error(ErrorCode::INVALID_NODE_STATE);
        }
        if(this->isInReplicationMode() && this->isInReplicationMode() && options.requestOfNodeToReplicate &&
            !this->cluster->getPartitionObject()->canHoldKey(request.operation.args->at(0))){
            return Response::error(ErrorCode::INVALID_PARTITION);
        }
        if(this->isInReplicationMode() && options.requestOfNodeToReplicate &&
           !NodeStates::cantExecuteRequest(this->cluster->getNodeState())){
            this->replicationOperationBuffer->add(request);
            return Response::success();
        }

        Response result = this->executeOperator(operatorToExecute, request.operation, options);

        return result;
    }

    Response executeOperator(std::shared_ptr<Operator> operatorToExecute,
                    const OperationBody& operation,
                    const OperationOptions& options) {

        OperatorDependencies dependencies = this->getDependencies(operatorToExecute);
        Response result = operatorToExecute->operate(operation, options, dependencies);

        this->logger->debugInfo("Executed {0} append request for operator {1} from {2}",
                                result.isSuccessful ? "successfuly" : "unsuccessfuly",
                                operatorToExecute->name(), options.requestOfNodeToReplicate ? "node" : "user");

        if(operatorToExecute->type() == WRITE && result.isSuccessful) {
            this->operationLog->add(operation);

            if(!options.requestOfNodeToReplicate) {
                result.timestamp = this->clock->tick(operation.timestamp);
            }

            if(this->isInReplicationMode() && !options.requestOfNodeToReplicate){
                this->cluster->broadcast(operation);

                this->logger->debugInfo("Broadcasted request for operator {0} from {1}",
                                        operatorToExecute->name(), options.requestOfNodeToReplicate ? "node" : "user");
            }
        }

        return result;
    }

    void applyReplicatedOperationBuffer() {
        while(!this->replicationOperationBuffer->isEmpty()){
            Request operation = this->replicationOperationBuffer->get();
            this->dispatch(operation);
        }
    }

private:
    OperatorDependencies getDependencies(std::shared_ptr<Operator> operatorToGetDependecies) {
        OperatorDependencies dependencies;
        
        for(auto dependency : operatorToGetDependecies->dependencies()){
            this->getDependecy(dependency, &dependencies);
        }

        return dependencies;
    }

    void getDependecy(OperatorDependency dependency, OperatorDependencies * operatorDependencies){
        switch (dependency) {
            case OPERATION_LOG:
                operatorDependencies->operationLog = this->operationLog;
                break;
            case DB_STORE:
                operatorDependencies->dbStore = this->db;
                break;
            case CLUSTER:
                operatorDependencies->cluster = this->cluster;
                break;
        }
    }
    
    void applyReplicatedOperationBufferIfNotEmpty() {
        if(!this->replicationOperationBuffer->isEmpty()){
            this->applyReplicatedOperationBuffer();
        }
    }

    bool isAuthorizedToExecute(std::shared_ptr<Operator> operatorToExecute, AuthenticationType authenticationOfUser) {
        for(AuthenticationType authentationTypeRequiredForOperator : operatorToExecute->authorizedToExecute()) {
            if(authentationTypeRequiredForOperator == authenticationOfUser){
                return true;
            }
        }

        return false;
    }

    bool isInReplicationMode() {
        return this->configuration->getBoolean(ConfigurationKeys::MEMDB_CORE_USE_REPLICATION);
    }

    bool isInPartitionMode() {
        return this->configuration->getBoolean(ConfigurationKeys::MEMDB_CORE_USE_PARTITIONS);
    }
};

using operatorDispatcher_t = std::shared_ptr<OperatorDispatcher>;