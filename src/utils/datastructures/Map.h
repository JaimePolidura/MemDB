#pragma once

#include "optional"
#include "string"

struct MapEntry {
    char * value;
    size_t valueSize;
};

class Map {
    virtual void put(const std::string& key, char * value, size_t valueSize) = 0;

    virtual std::optional<MapEntry> get(const std::string& key) = 0;

    virtual std::optional<MapEntry> remove(const std::string &key) = 0;

    virtual bool contains(const std::string &key) = 0;

    virtual int getSize() = 0;
};