#include "operators/AllOperators.h"

std::vector<std::shared_ptr<Operator>> AllOperators::list() {
    return {
            std::make_shared<GetOperator>(),
            std::make_shared<SetOperator>(),
            std::make_shared<DeleteOperator>(),
            std::make_shared<SyncOplogOperator>(),
            std::make_shared<HealthCheckOperator>(),
    };
}