#pragma once

#include "shared.h"

class Utils {
public:
    template<typename T>
    static std::vector<T> concat(const std::vector<T>& a, const std::vector<T>& b) {
        if(a.empty() && b.empty()) {
            std::cout << "b" << std::endl;
            return std::vector<T>{};
        }
        if(a.empty())
            return b;
        if(b.empty())
            return a;

        std::cout << "e" << std::endl;

        std::vector<T> concatenated;
        concatenated.reserve(a.size() + b.size());
        concatenated.insert(concatenated.end(), a.begin(), a.end());
        concatenated.insert(concatenated.end(), b.begin(), b.end());

        return concatenated;
    }

    static bool tryOnce(std::function<void(void)> toTry) {
        try{
            toTry();
            return true;
        }catch (const std::exception& e) {
            return false;
        }
    }

    static void printVector(const std::vector<uint8_t>& toPrint) {
        std::cout << "[";

        for (size_t i = 0; i < toPrint.size(); ++i) {
            std::cout << static_cast<unsigned>(toPrint[i]);
            if (i != toPrint.size() - 1)
                std::cout << ", ";
        }
        std::cout << "]" << std::endl;
    }

    template<typename T, typename B>
    static std::optional<T> retryUntilAndGet(int numberAttempts, const std::chrono::duration<int64_t, B> backoffMillis, std::function<T(void)> toRetry) {
        while(numberAttempts > 0) {
            try{
                return toRetry();
            }catch (const std::exception& e){
                numberAttempts--;

                if(backoffMillis.count() > 0)
                    std::this_thread::sleep_for(backoffMillis);
            }
        }

        return std::nullopt;
    }

    template<typename R, typename P>
    static bool retryUntil(int numberAttempts, const std::chrono::duration<R, P> backoff, std::function<void(void)> toRetry) {
        while(numberAttempts > 0) {
            try{
                toRetry();

                return true;
            }catch (const std::exception& e){
                numberAttempts--;

                if(backoff.count() > 0)
                    std::this_thread::sleep_for(backoff);
            }
        }

        return false;
    }

    template<typename T>
    static T parse(const uint8_t * input) {
        T parsedValue = 0;
        for(int i = 0; i < sizeof(T); i++) {
            size_t toMove = (sizeof(T) - 1 - i) * 8;
            parsedValue |= static_cast<T>(input[i]) << toMove;
        }

        return parsedValue;
    }

    template<typename T>
    static T parseFromBuffer(const std::vector<uint8_t>& buffer, const uint8_t initialPosition = 0) {
        T result = 0;
        for(int i = sizeof(T) - 1; i >= 0; i--){
            size_t toMove = ((sizeof(T) - 1) - i) * 8;
            result |= buffer[i + initialPosition] << toMove;            
        }

        return result;
    }

    template<typename T>
    static std::vector<uint8_t> appendToBuffer(const T& toParse, std::vector<uint8_t>& vectorOut) {
        for(int i = 0; i < sizeof(T); i++){
            size_t tomove = ((sizeof(T) - 1) - i) * 8;
            vectorOut.push_back(static_cast<uint8_t>((toParse >> tomove)));
        }

        return vectorOut;
    }

    static auto copyFromBuffer(const std::vector<uint8_t>& buffer, const uint8_t initialPosition, const uint8_t endPosition) -> uint8_t * {
        uint8_t * copyDest = new uint8_t[endPosition - initialPosition + 1];
        std::copy(buffer.begin() + initialPosition, buffer.begin() + endPosition + 1, copyDest);

        return copyDest;
    }

    static std::vector<uint8_t> appendToBuffer(const uint8_t * toAppendPtr, const uint8_t totalToAppend, std::vector<uint8_t>& vectorOut) {
        for(auto i = 0; i < totalToAppend; i++) {
            vectorOut.push_back(* (toAppendPtr + i));
        }

        return vectorOut;
    }
};