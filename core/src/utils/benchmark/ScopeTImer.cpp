#include "utils/benchmark/ScopeTimer.h"

ScopeTimer::ScopeTimer(std::string name):
    name(std::move(name)),
    stoped(false),
    start_milliseconds(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()
                                                                                 .time_since_epoch()).count()) {}
void ScopeTimer::stop() {
    this->stoped = true;
    this->printResult();
}

void ScopeTimer::printResult() {
    uint64_t end_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()
                                                                                              .time_since_epoch()).count();
    uint64_t difference = end_milliseconds - start_milliseconds;

    std::cout << this->name << " duration: " << difference << " ms" << std::endl;
}