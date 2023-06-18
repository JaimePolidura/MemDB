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
    ScopeTimer(std::string name):
            name(std::move(name)),
            stoped(false),
            start_milliseconds(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()
                .time_since_epoch()).count()) {}

    ~ScopeTimer() {
        if(!this->stoped)
            this->printResult();
    }

    void stop() {
        this->stoped = true;
        this->printResult();
    }

private:
    void printResult() {
        uint64_t end_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()
                                                                                                  .time_since_epoch()).count();
        uint64_t difference = end_milliseconds - start_milliseconds;

        std::cout << this->name << " duration: " << difference << " ms" << std::endl;
    }
};