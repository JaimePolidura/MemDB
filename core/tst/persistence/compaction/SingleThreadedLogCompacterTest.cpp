#include "gtest/gtest.h"

#include "persistence/compaction/singlethreaded/SingleThreadedLogCompacter.h"
#include "CompactionTestUtils.h"
#include "../../Assertions.h"

TEST(SingleThreadedLogCompacter, ShouldCompact) {
    SingleThreadedLogCompacter singleThreadedLogCompacter{};

    std::vector<OperationBody> compacted = singleThreadedLogCompacter.compact({
        CompactionTestUtils::set('A', '1'),
        CompactionTestUtils::set('B', '2'),
        CompactionTestUtils::del('A'),
        CompactionTestUtils::set('B', '3'),
        CompactionTestUtils::del('C'),
        CompactionTestUtils::set('A', '0')
    });

    //Expected: S(B, 3) D(C) S(A, 0);

    ASSERT_EQ(compacted.size(), 3);

    Assertions::assertAnyMatch<OperationBody>(compacted, [](const OperationBody& op) -> bool {
        return op.operatorNumber == 2 && *op.args->at(0).data() == 'C';
    });

    Assertions::assertAnyMatch<OperationBody>(compacted, [](const OperationBody& op) -> bool {
        return op.operatorNumber == 1 && *op.args->at(0).data() == 'B' && *op.args->at(1).data() == '3';
    });

    Assertions::assertAnyMatch<OperationBody>(compacted, [](const OperationBody& op) -> bool {
        return op.operatorNumber == 1 && *op.args->at(0).data() == 'A' && *op.args->at(1).data() == '0';
    });
}