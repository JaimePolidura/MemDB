#pragma once

#include "shared.h"

class Utils {
public:
    template<typename K, typename V>
    static std::vector<V> collectValuesInto(const std::map<K, V>& map, std::vector<V>& vector) {
        for (const auto& entry : map) {
            vector.push_back(entry.second);
        }

        return vector;
    }

    template<typename K, typename V>
    static std::map<K, V> unionMaps(const std::map<K, V>& first, const std::map<K, V>& second) {
        std::map<K, V> toReturn{};

        for(const auto& [keyFirst, valueFirst] : first){
            toReturn[keyFirst] = valueFirst;
        }
        for(const auto& [keySecond, valueSecond] : second){
            toReturn[keySecond] = valueSecond;
        }

        return toReturn;
    }

    template<typename T>
    static std::vector<T> filter(const std::vector<T>& initial, std::function<bool(const T&)> predicate) {
        std::vector<T> result{};
        result.reserve(initial.size());

        for (const T& item : initial) {
            if(predicate(item)){
                result.push_back(item);
            }
        }

        return result;
    }
    
    template<typename T>
    static std::vector<T> concat(const std::vector<T>& a, const std::vector<T>& b) {
        if(a.empty() && b.empty()) {
            return std::vector<T>{};
        }
        if(a.empty())
            return b;
        if(b.empty())
            return a;

        std::vector<T> concatenated;
        concatenated.reserve(a.size() + b.size());
        concatenated.insert(concatenated.end(), a.begin(), a.end());
        concatenated.insert(concatenated.end(), b.begin(), b.end());

        return concatenated;
    }

    template<typename T>
    inline static T optimizedModulePowerOfTwo(T powerOfTwo, T num) {
        return (powerOfTwo - 1) & num;
    }

    template<typename T>
    static T roundUpPowerOfTwo(T initial) {
        if(initial <= 1){
            return 1;
        }

        initial--;
        initial |= initial >> 1;
        initial |= initial >> 2;
        initial |= initial >> 4;
        initial |= initial >> 8;
        initial |= initial >> 16;
        initial++;

        return initial;
    }

    static bool tryOnce(std::function<void(void)> toTry);

    static void printVector(const std::vector<uint8_t>& toPrint);

    static void printVectorHex(const std::vector<uint8_t>& toPrint);

    template<typename T, typename B>
    static T retryUntilSuccessAndGet(const std::chrono::duration<int64_t, B> backoffMillis, std::function<T(void)> toRetry) {
        while(true) {
            try{
                return toRetry();
            }catch (const std::exception& e){
                if(backoffMillis.count() > 0)
                    std::this_thread::sleep_for(backoffMillis);
            }
        }
    }

    template<typename T>
    static std::optional<T> tryOnceAndGetOptional(std::function<std::optional<T>()> toRetry) {
        try{
            return toRetry();
        }catch(const std::exception& e) {
            return std::nullopt;
        }
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

    template<typename T>
    static T getOptionalOrThrow(const std::optional<T>& optional, const std::string& exceptionMessage = "No _value for optional") {
        if(optional.has_value()) {
            return optional.value();
        } else {
            throw std::runtime_error(exceptionMessage);
        }
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
            uint8_t value = input[i];
            parsedValue |= static_cast<T>(value) << toMove;
        }

        return parsedValue;
    }

    template<typename T>
    static T parseFromBuffer(const std::vector<uint8_t>& buffer, const uint64_t initialPosition = 0) {
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
            size_t toMove = ((sizeof(T) - 1) - i) * 8;
            vectorOut.push_back(static_cast<uint8_t>((toParse >> toMove)));
        }

        return vectorOut;
    }

    template<typename T>
    static std::vector<uint8_t> appendBeginningToBuffer(const T& toParse, std::vector<uint8_t>& vectorOut) {
        for(int i = 0; i < sizeof(T); i++){
            size_t toMove = ((sizeof(T) - 1) - i) * 8;
            vectorOut.insert(vectorOut.begin() + i, static_cast<uint8_t>(toParse >> toMove));
        }

        return vectorOut;
    }

    static auto copyFromBuffer(const std::vector<uint8_t>& buffer, const uint64_t initialPosition, const uint64_t endPosition) -> uint8_t *;

    static std::vector<uint8_t> appendToBuffer(const uint8_t * toAppendPtr, const uint64_t totalToAppend, std::vector<uint8_t>& vectorOut);

    static uint32_t crc(const std::vector<uint8_t>& values);
};