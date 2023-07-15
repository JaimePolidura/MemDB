#pragma once

#include <chrono>
#include <iostream>

#define TIME_THIS_SCOPE(name) ScopeTimer(name)

class ScopeTimer {
private:
    uint64_t start_milliseconds;
    std::string name;
    bool stoped;

public:
    ScopeTimer(std::string name);

    ~ScopeTimer() {
        if(!this->stoped)
            this->printResult();
    }

    void stop();

private:
    void printResult();
};