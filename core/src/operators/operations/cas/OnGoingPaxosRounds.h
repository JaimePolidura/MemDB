#pragma once

#include "shared.h"

enum PaxosRole {
    ACCEPTATOR,
    PROPOSSER
};

enum PaxosState {
    PROMISED,
    ACCEPTED
};

struct PaxosRound {
    PaxosState state;
    PaxosRole role;

    LamportClock promisedTimestamp;
    LamportClock acceptedTimestamp;

    SimpleString<memDbDataLength_t> value;

    std::mutex lock;
};

class OnGoingPaxosRounds {
private:
    std::map<uint32_t, PaxosRound> paxosRoundsByKeyHash{};

public:
    void registerNewPaxosRoundPromised(uint32_t keyHash, LamportClock promisedTimestamp);
    void registerNewPaxosRoundAccept(uint32_t keyHash, LamportClock promisedTimestamp);

    void updatePromisedTimestamp(uint32_t keyHash, LamportClock promisedTimestamp);
    void updateAcceptedTimestamp(uint32_t keyHash, LamportClock promisedTimestamp);

    std::optional<PaxosRound> getRoundByKeyHash(uint32_t keyHash);
};

using onGoingPaxosRounds_t = std::shared_ptr<OnGoingPaxosRounds>;