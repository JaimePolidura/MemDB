#pragma once

struct SendRequestOptions {
    int partitionId{0};
    bool canBeStoredInHint{false};
};