#include "OnGoingPaxosRounds.h"

bool OnGoingPaxosRounds::isOnGoingProposer(uint32_t keyHash) {
    std::unique_lock<std::mutex> uniqueLock(this->proposerLock);

    if(this->paxosRoundsProposerByKeyHash.contains(keyHash)){
        ProposerPaxosRound& paxosRound = this->paxosRoundsProposerByKeyHash.at(keyHash);
        return paxosRound.state == ProposerPaxosState::WAITING_FOR_ACCEPT || paxosRound.state == ProposerPaxosState::WAITING_FOR_PROMISE;
    } else {
        return false;
    }
}

ProposerPaxosRound OnGoingPaxosRounds::getProposerByKeyHashOrCreate(uint32_t keyHash, ProposerPaxosState newState) {
    if(!this->paxosRoundsProposerByKeyHash.contains(keyHash)){
        ProposerPaxosRound paxosRound = ProposerPaxosRound{.state = newState, .retryPrepareTimer = ExponentialRandomizedTimer{1000, 1000 * 60}};
        this->paxosRoundsProposerByKeyHash.insert({keyHash, paxosRound});
        return paxosRound;
    } else {
        return this->paxosRoundsProposerByKeyHash.at(keyHash);
    }
}

void OnGoingPaxosRounds::updateStateProposer(uint32_t keyHash, ProposerPaxosState newState) {
    std::unique_lock<std::mutex> uniqueLock(this->proposerLock);

    if(!this->paxosRoundsProposerByKeyHash.contains(keyHash)) {
        this->paxosRoundsProposerByKeyHash.insert({keyHash, ProposerPaxosRound{.state = newState, .retryPrepareTimer = ExponentialRandomizedTimer{1000, 1000 * 60}}});
    } else {
        ProposerPaxosRound& paxosRound = this->paxosRoundsProposerByKeyHash.at(keyHash);
        paxosRound.state = newState;
    }
}

void OnGoingPaxosRounds::updateAcceptedTimestamp(uint32_t keyHash, LamportClock promisedTimestamp) {
    std::unique_lock<std::mutex> uniqueLock(this->proposerLock);

    AcceptatorPaxosRound& paxosRound = this->paxosRoundsAcceptatorByKeyHash.at(keyHash);
    paxosRound.promisedNextTimestamp = promisedTimestamp;
}

void OnGoingPaxosRounds::updatePromisedTimestamp(uint32_t keyHash, LamportClock promisedTimestamp) {
    std::unique_lock<std::mutex> uniqueLock(this->proposerLock);

    AcceptatorPaxosRound& paxosRound = this->paxosRoundsAcceptatorByKeyHash.at(keyHash);
    paxosRound.promisedNextTimestamp = promisedTimestamp;
}

void OnGoingPaxosRounds::registerNewAcceptatorPromise(uint32_t keyHash, LamportClock nextTimestamp) {
    std::unique_lock<std::mutex> uniqueLock(this->acceptatorLock);
    this->paxosRoundsAcceptatorByKeyHash[keyHash] = AcceptatorPaxosRound{
        .state = AcceptatorPaxosState::ACCEPTATOR_PROMISED,
        .promisedNextTimestamp = nextTimestamp
    };
}

std::optional<AcceptatorPaxosRound> OnGoingPaxosRounds::getAcceptatorByKeyHash(uint32_t keyHash) {
    std::unique_lock<std::mutex> uniqueLock(this->acceptatorLock);
    return this->paxosRoundsAcceptatorByKeyHash.contains(keyHash) ?
           std::optional<AcceptatorPaxosRound>{this->paxosRoundsAcceptatorByKeyHash.at(keyHash)} :
           std::nullopt;
}