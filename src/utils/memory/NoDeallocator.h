#pragma once

#include <cstdint>

template<typename T>
class NoDeallocator {
public:
    using value_type = T;

    NoDeallocator() = default;

    template <typename U>
    NoDeallocator(const NoDeallocator<U>&) {}

    T * allocate(std::size_t n) {
        return static_cast<T *>(::operator new(n * sizeof(T)));
    }

    void deallocate(T*, std::size_t) {}
};