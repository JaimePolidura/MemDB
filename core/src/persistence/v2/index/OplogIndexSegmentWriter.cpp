#include "OplogIndexSegmentWriter.h"

OplogIndexSegmentWriter::OplogIndexSegmentWriter(const std::string& partitionPath): partitionPath(partitionPath) {}

void OplogIndexSegmentWriter::write(const std::vector<uint8_t> &toWrite) {
}