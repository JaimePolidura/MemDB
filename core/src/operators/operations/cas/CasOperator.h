#pragma once

#include "operators/Operator.h"
#include "messages/response/ResponseBuilder.h"
#include "messages/response/ErrorCode.h"
#include "operators/operations/cas/OnGoingPaxosRounds.h"

/**
 * Args:
 *  - uint32 Key
 *  - uint32 Expected value
 *  - uint32 New value
 */
class CasOperator : public Operator {
public:
    Response operate(const OperationBody& operation, const OperationOptions operationOptions, OperatorDependencies& dependencies) override;

    OperatorDescriptor desc() override;

private:
    std::tuple<SimpleString<memDbDataLength_t>,
            SimpleString<memDbDataLength_t>,
            SimpleString<memDbDataLength_t>> getArgs(const OperationBody& operation);

    std::result<std::tuple<LamportClock, LamportClock>> sendRetriesPrepares(OperatorDependencies& dependencies,
                                                                            proposerPaxosRound_t proposerPaxosRound,
                                                                            int partitionId,
                                                                            SimpleString<memDbDataLength_t> key,
                                                                            SimpleString<memDbDataLength_t> expectedValue);

    bool checkIfQuorumAndAllResponsesSuccess(multipleResponses_t multiResponses,
                                             proposerPaxosRound_t proposerPaxosRound,
                                             OperatorDependencies& dependencies);

    LamportClock getNextTimestampForKey(MapEntry<memDbDataLength_t> mapEntry,
                                       OperatorDependencies& dependencies);

    multipleResponses_t sendPrepare(OperatorDependencies& dependencies,
                                    proposerPaxosRound_t proposerPaxosRound,
                                    int partitionId,
                                    SimpleString<memDbDataLength_t> key,
                                    LamportClock prevTimestamp,
                                    LamportClock nextTimestamp);

    multipleResponses_t sendAccept(OperatorDependencies& dependencies,
                                   proposerPaxosRound_t proposerPaxosRound,
                                   int partitionId,
                                   SimpleString<memDbDataLength_t> key,
                                   SimpleString<memDbDataLength_t> value,
                                   LamportClock prevTimestamp,
                                   LamportClock nextTimestamp);
};