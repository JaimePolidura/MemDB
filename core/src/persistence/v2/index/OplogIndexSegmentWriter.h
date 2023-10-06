#pragma once

#include "OplogIndexSegmentDescriptor.h"

class OplogIndexSegmentWriter {
private:
    const std::string partitionPath;

public:
    OplogIndexSegmentWriter(const std::string& partitionPath);

    void write(const std::vector<uint8_t>& toWrite);
};
