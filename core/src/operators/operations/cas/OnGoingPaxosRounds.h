#pragma once

#include "shared.h"
#include "utils/clock/LamportClock.h"

enum PaxosRole {
    ACCEPTATOR,
    PROPOSSER
};

enum PaxosState {
    PROPOSER_WAITING_FOR_PROMISE,
    PROPOSER_WAITING_FOR_ACCEPT,
    PROPOSER_COMITTED,
    PROPOSER_FAILED,

    ACCEPTATOR_PROMISED,
};

struct PaxosRound {
    PaxosState state;
    PaxosRole role;

    LamportClock acceptatorPromisedNextTimestamp;
    LamportClock acceptatorAcceptedNextTimestamp;
};

class OnGoingPaxosRounds {
private:
    std::unordered_map<uint32_t, PaxosRound> paxosRoundsProposerByKeyHash{};
    std::mutex proposerLock{};

    std::unordered_map<uint32_t, PaxosRound> paxosRoundsAcceptatorByKeyHash{};
    std::mutex acceptatorLock{};

public:
    bool isPaxosRoundOnGoingProposer(uint32_t keyHash);
    void updatePaxosRoundStateProposer(uint32_t keyHash, PaxosState newState);

    std::optional<PaxosRound> getAcceptatorRoundByKeyHash(uint32_t keyHash);
    void registerNewAcceptatorPromisePaxosRound(uint32_t keyHash, LamportClock nextTimestamp);
    void updatePromisedTimestamp(uint32_t keyHash, LamportClock promisedTimestamp);
    void updateAcceptedTimestamp(uint32_t keyHash, LamportClock promisedTimestamp);


    void registerNewPaxosRoundPromised(uint32_t keyHash, LamportClock promisedTimestamp);

    void updateAcceptedTimestamp(uint32_t keyHash, LamportClock promisedTimestamp, SimpleString<memDbDataLength_t> value);

};

using onGoingPaxosRounds_t = std::shared_ptr<OnGoingPaxosRounds>;