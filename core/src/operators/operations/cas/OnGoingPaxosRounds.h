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
    std::mutex proposerLock{};

    ProposerPaxosRound(ProposerPaxosState state, ExponentialRandomizedTimer retryPrepareTimer):
        state(state), retryPrepareTimer(retryPrepareTimer) {}
};

using proposerPaxosRound_t = std::shared_ptr<ProposerPaxosRound>;

struct AcceptatorPaxosRound {
    AcceptatorPaxosState state;

    LamportClock promisedNextTimestamp{0, 0};
};

class OnGoingPaxosRounds {
private:
    std::unordered_map<uint32_t, proposerPaxosRound_t> paxosRoundsProposerByKeyHash{};
    std::mutex proposerLock{};

    std::unordered_map<uint32_t, AcceptatorPaxosRound> paxosRoundsAcceptatorByKeyHash{};
    std::mutex acceptatorLock{};

public:
    //Proposer
    proposerPaxosRound_t getProposerByKeyHash(uint32_t keyHash, ProposerPaxosState newStateIfNotFound);

    //Acceptator
    std::optional<AcceptatorPaxosRound> getAcceptatorByKeyHash(uint32_t keyHash);
    void registerNewAcceptatorPromise(uint32_t keyHash, LamportClock nextTimestamp);
    void updatePromisedTimestamp(uint32_t keyHash, LamportClock promisedTimestamp);
    void updateAcceptedTimestamp(uint32_t keyHash, LamportClock promisedTimestamp);
};

using onGoingPaxosRounds_t = std::shared_ptr<OnGoingPaxosRounds>;