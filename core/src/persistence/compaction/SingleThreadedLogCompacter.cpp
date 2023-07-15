#include "persistence/compaction/SingleThreadedLogCompacter.h"

auto SingleThreadedLogCompacter::compact(const std::vector<OperationBody>& uncompacted,
             const std::vector<OperationBody>& compacted,
             const setSimpleString_t& seenOperationKeys) -> std::vector<OperationBody> {
    if(uncompacted.empty()) {
        return uncompacted;
    }

    for(int i = uncompacted.size() - 1; i >= 0; i--) {
        auto actualOperation = uncompacted[i];
        auto actualOperationKey = * actualOperation.args->begin();

        if(seenOperationKeys.contains(actualOperationKey)){
            continue;
        }

        const_cast<setSimpleString_t&>(seenOperationKeys).insert(actualOperationKey);
        const_cast<std::vector<OperationBody>&>(compacted).push_back(actualOperation);
    }

    return compacted;
}