#include "persistence/compaction/OperationLogCompacter.h"

std::vector<OperationBody> OperationLogCompacter::compact(const std::vector<OperationBody>& uncompacted) {
    return uncompacted.size() > 10.000 ?
           this->multiThreaded.compact(std::make_shared<std::vector<OperationBody>>(uncompacted)) :
           this->singleThreaded.compact(uncompacted);
}