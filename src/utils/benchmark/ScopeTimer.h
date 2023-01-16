#include <chrono>
#include <iostream>

#define TIME_THIS_SCOPE(name) ScopeTimer(name)

class ScopeTimer {
private:
    uint64_t start_microseconds;
    char * name;

public:
    ScopeTimer(char * name):
        name(name),
        start_microseconds(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now()
            .time_since_epoch()).count()) {}

    ~ScopeTimer() {
        uint64_t end_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now()
                .time_since_epoch()).count();

        uint64_t difference = start_microseconds - end_microseconds;

        std::cout << this->name << " duration: " << difference << " ms" << std::endl;
    }
};