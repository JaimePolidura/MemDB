#include "OnGoingPaxosRounds.h"

bool OnGoingPaxosRounds::isPaxosRoundOnGoingProposer(uint32_t keyHash) {
    std::unique_lock<std::mutex> uniqueLock(this->proposerLock);

    if(this->paxosRoundsProposerByKeyHash.contains(keyHash)){
        PaxosRound& paxosRound = this->paxosRoundsProposerByKeyHash.at(keyHash);
        return paxosRound.state == PaxosState::PROPOSER_WAITING_FOR_ACCEPT || paxosRound.state == PaxosState::PROPOSER_WAITING_FOR_PROMISE;
    } else {
        return false;
    }
}

void OnGoingPaxosRounds::updatePaxosRoundStateProposer(uint32_t keyHash, PaxosState newState) {
    std::unique_lock<std::mutex> uniqueLock(this->proposerLock);

    if(!this->paxosRoundsProposerByKeyHash.contains(keyHash)) {
        this->paxosRoundsProposerByKeyHash[keyHash] = PaxosRound{.state = newState, .role = PaxosRole::PROPOSSER};
    } else {
        PaxosRound& paxosRound = this->paxosRoundsProposerByKeyHash.at(keyHash);
        paxosRound.state = newState;
    }
}

void OnGoingPaxosRounds::updateAcceptedTimestamp(uint32_t keyHash, LamportClock promisedTimestamp) {
    std::unique_lock<std::mutex> uniqueLock(this->proposerLock);

    PaxosRound& paxosRound = this->paxosRoundsAcceptatorByKeyHash.at(keyHash);
    paxosRound.acceptatorAcceptedNextTimestamp = promisedTimestamp;
}

void OnGoingPaxosRounds::updatePromisedTimestamp(uint32_t keyHash, LamportClock promisedTimestamp) {
    std::unique_lock<std::mutex> uniqueLock(this->proposerLock);

    PaxosRound& paxosRound = this->paxosRoundsAcceptatorByKeyHash.at(keyHash);
    paxosRound.acceptatorPromisedNextTimestamp = promisedTimestamp;
}

void OnGoingPaxosRounds::registerNewAcceptatorPromisePaxosRound(uint32_t keyHash, LamportClock nextTimestamp) {
    std::unique_lock<std::mutex> uniqueLock(this->acceptatorLock);
    this->paxosRoundsAcceptatorByKeyHash[keyHash] = PaxosRound{
        .state = PaxosState::ACCEPTATOR_PROMISED,
        .acceptatorPromisedNextTimestamp = nextTimestamp
    };
}

std::optional<PaxosRound> OnGoingPaxosRounds::getAcceptatorRoundByKeyHash(uint32_t keyHash) {
    std::unique_lock<std::mutex> uniqueLock(this->acceptatorLock);
    return this->paxosRoundsAcceptatorByKeyHash.contains(keyHash) ?
           std::optional<PaxosRound>{this->paxosRoundsAcceptatorByKeyHash.at(keyHash)} :
           std::nullopt;
}