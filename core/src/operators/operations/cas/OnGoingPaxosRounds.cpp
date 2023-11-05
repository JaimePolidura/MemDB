#include "OnGoingPaxosRounds.h"

proposerPaxosRound_t OnGoingPaxosRounds::getProposerByKeyHash(uint32_t keyHash, ProposerPaxosState newStateIfNotFound) {
    if(!this->paxosRoundsProposerByKeyHash.contains(keyHash)){
        proposerPaxosRound_t paxosRound = std::make_shared<ProposerPaxosRound>(newStateIfNotFound, ExponentialRandomizedTimer{1000, 1000 * 60});
        this->paxosRoundsProposerByKeyHash.insert({keyHash, paxosRound});
        return paxosRound;
    } else {
        return this->paxosRoundsProposerByKeyHash.at(keyHash);
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