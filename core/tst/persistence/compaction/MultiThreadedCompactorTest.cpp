#include "gtest/gtest.h"

#include "persistence/compaction/multithreaded/MultithreadedLogCompactor.h"
#include "CompactionTestUtils.h"
#include "../../Assertions.h"

/**
 * 25 ops
 * 8 threads
 * -> 8 initial blocks blocks
 * -> heigth 3
 *
 * Expected result: SET(H, 1) DEL(A) SET(B, 4) SET(E, 4) SET(C, 2) DEL(F) SET(D, 1)
 */
TEST(MultiThreadedLogCompactor, ShouldCompact) {
    std::vector<OperationBody> * uncompacted = new std::vector<OperationBody>{
        CompactionTestUtils::set('A', '1'),
        CompactionTestUtils::set('B', '2'),
        CompactionTestUtils::set('A', '3'),
        CompactionTestUtils::del('C'),
        CompactionTestUtils::set('D', '1'),
        CompactionTestUtils::set('A', '4'),
        CompactionTestUtils::set('E', '1'),
        CompactionTestUtils::del('F'),
        CompactionTestUtils::set('C', '2'),
        CompactionTestUtils::set('A', '3'),
        CompactionTestUtils::set('E', '4'),
        CompactionTestUtils::set('B', '1'),
        CompactionTestUtils::del('B'),
        CompactionTestUtils::set('A', '6'),
        CompactionTestUtils::set('B', '9'),
        CompactionTestUtils::del('A'),
        CompactionTestUtils::set('A', '3'),
        CompactionTestUtils::del('A'),
        CompactionTestUtils::set('A', '2'),
        CompactionTestUtils::set('B', '4'),
        CompactionTestUtils::del('A'),
        CompactionTestUtils::set('A', '1'),
        CompactionTestUtils::del('A'),
        CompactionTestUtils::del('H'),
        CompactionTestUtils::set('H', '1'),
    };

    auto compacted = MultiThreadedLogCompactor::compact(std::shared_ptr<std::vector<OperationBody>>{uncompacted});

    ASSERT_EQ(compacted.size(), 7);

    Assertions::assertAnyMatch<OperationBody>(compacted, [](const auto& op) -> bool {
        return *op.args->at(0).data() == 'H' && op.number == 1 && *op.args->at(1).data() == '1';
    });

    Assertions::assertAnyMatch<OperationBody>(compacted, [](const auto& op) -> bool {
        return *op.args->at(0).data() == 'A' && op.number == 2;
    });

    Assertions::assertAnyMatch<OperationBody>(compacted, [](const auto& op) -> bool {
        return *op.args->at(0).data() == 'H' && op.number == 1 && *op.args->at(1).data() == '1';
    });

    Assertions::assertAnyMatch<OperationBody>(compacted, [](const auto& op) -> bool {
        return *op.args->at(0).data() == 'E' && op.number == 1 && *op.args->at(1).data() == '4';
    });

    Assertions::assertAnyMatch<OperationBody>(compacted, [](const auto& op) -> bool {
        return *op.args->at(0).data() == 'C' && op.number == 1 && *op.args->at(1).data() == '2';
    });

    Assertions::assertAnyMatch<OperationBody>(compacted, [](const auto& op) -> bool {
        return *op.args->at(0).data() == 'F' && op.number == 2;
    });

    Assertions::assertAnyMatch<OperationBody>(compacted, [](const auto& op) -> bool {
        return *op.args->at(0).data() == 'D' && op.number == 1 && *op.args->at(1).data() == '1';
    });
}