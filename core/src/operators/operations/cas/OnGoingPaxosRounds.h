#pragma once

#include "shared.h"
#include "utils/clock/ExponentialRandomizedTimer.h"
#include "utils/clock/LamportClock.h"

enum ProposerPaxosState {
    WAITING_FOR_PROMISE,
    WAITING_FOR_ACCEPT,
    COMITTED,
    FAILED,
};

enum AcceptatorPaxosState {
    ACCEPTATOR_PROMISED,
};

struct ProposerPaxosRound {
    ProposerPaxosState state;

    ExponentialRandomizedTimer retryPrepareTimer;
};

struct AcceptatorPaxosRound {
    AcceptatorPaxosState state;

    LamportClock promisedNextTimestamp;
    LamportClock promisedOldTimestamp;
};

class OnGoingPaxosRounds {
private:
    std::unordered_map<uint32_t, ProposerPaxosRound> paxosRoundsProposerByKeyHash{};
    std::mutex proposerLock{};

    std::unordered_map<uint32_t, AcceptatorPaxosRound> paxosRoundsAcceptatorByKeyHash{};
    std::mutex acceptatorLock{};

public:
    //Proposer
    bool isOnGoingProposer(uint32_t keyHash);
    void updateStateProposer(uint32_t keyHash, ProposerPaxosState newState);
    ProposerPaxosRound getProposerByKeyHashOrCreate(uint32_t keyHash, ProposerPaxosState newState);

    //Acceptator
    std::optional<AcceptatorPaxosRound> getAcceptatorByKeyHash(uint32_t keyHash);
    void registerNewAcceptatorPromise(uint32_t keyHash, LamportClock nextTimestamp);
    void updatePromisedTimestamp(uint32_t keyHash, LamportClock promisedTimestamp);
    void updateAcceptedTimestamp(uint32_t keyHash, LamportClock promisedTimestamp);
};

using onGoingPaxosRounds_t = std::shared_ptr<OnGoingPaxosRounds>;