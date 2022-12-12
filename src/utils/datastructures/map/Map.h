#pragma once

#include "optional"
#include "string"

struct MapEntry {
    uint8_t * value;
    size_t valueSize;
};

class Map {
public:
    virtual void put(const std::string& key, uint8_t * value, size_t valueSize) = 0;

    virtual std::optional<MapEntry> get(const std::string& key) const = 0;

    virtual std::optional<MapEntry> remove(const std::string &key) = 0;

    virtual bool contains(const std::string &key) const = 0;

    virtual int getSize() const = 0;
};