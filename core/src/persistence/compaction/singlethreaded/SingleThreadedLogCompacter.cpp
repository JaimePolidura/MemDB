#include "SingleThreadedLogCompacter.h"

auto SingleThreadedLogCompacter::compact(
        const std::vector<OperationBody>& uncompacted,
        const std::vector<OperationBody>& compacted,
        const timestampsByKeysMap_t& timestampsByKeys) -> std::vector<OperationBody> {

    std::map<SimpleString<memDbDataLength_t>, OperationBody> compactedToReturn{};

    for(int i = uncompacted.size() - 1; i >= 0; i--) {
        auto actualOperation = uncompacted[i];
        auto actualOperationKey = * actualOperation.args->begin();
        auto actualTimestamp = actualOperation.timestamp;
        
        if(timestampsByKeys.contains(actualOperationKey)){
            if(timestampsByKeys.at(actualOperationKey) < actualTimestamp) {
                compactedToReturn[actualOperationKey] = actualOperation;
            }

            continue;
        }

        const_cast<timestampsByKeysMap_t&>(timestampsByKeys)[actualOperationKey] = actualTimestamp;
        compactedToReturn[actualOperationKey] = actualOperation;
    }

    return Utils::collectValuesInto(compactedToReturn, const_cast<std::vector<OperationBody>&>(compacted));
}