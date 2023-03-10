#pragma once

#include "shared.h"

#include "messages/request/Request.h"
#include "MultiThreadedCompactionBlock.h"

class MultiThreadedLogCompactor {
public:
    static std::vector<OperationBody> compact(allOperationLogs_t uncompacted) {
        return createCompactionTree(uncompacted)->get().get();
    }

private:
    static MultiThreadedCompactionBlock * createCompactionTree(allOperationLogs_t uncompacted) {
        auto blocksByPhase = createBlocksByPhaseMap(uncompacted);
        blocksByPhase = fillTreeChilds(blocksByPhase);

        return blocksByPhase[0].at(0);
    }

    static std::map<int, std::vector<MultiThreadedCompactionBlock *>> createBlocksByPhaseMap(allOperationLogs_t uncompacted) {
        uint32_t numberThreads = std::thread::hardware_concurrency();
        // We want the number of nodes that are goning to perform compaction to be the same as the number of threads. Min height nodes dont perform compaction
        uint32_t height = std::log2(numberThreads) + 2;
        std::map<int, std::vector<MultiThreadedCompactionBlock *>> blocksByPhase;

        for (int actualPhase = 0; actualPhase < height; actualPhase++) {
            blocksByPhase[actualPhase] = std::vector<MultiThreadedCompactionBlock * >{};
            int nodesToCreateInThisPhase = std::pow(2, actualPhase);
            bool firstPhase = actualPhase == 0;
            bool lastPhase = actualPhase + 1 == height;

            for(int j = 0; j < nodesToCreateInThisPhase; j++){
                if(firstPhase){
                    blocksByPhase[actualPhase].push_back(MultiThreadedCompactionBlock::root());
                }else if(lastPhase) {
                    blocksByPhase[actualPhase].push_back(MultiThreadedCompactionBlock::leaf(uncompacted, j, nodesToCreateInThisPhase));
                }else {
                    blocksByPhase[actualPhase].push_back(MultiThreadedCompactionBlock::node());
                }
            }
        }

        return blocksByPhase;
    }

    static std::map<int, std::vector<MultiThreadedCompactionBlock *>> fillTreeChilds(std::map<int, std::vector<MultiThreadedCompactionBlock *>>& blocksByPhase) {
        uint32_t numberThreads = std::thread::hardware_concurrency();
        uint32_t height = std::log2(numberThreads) + 2;

        for (const auto& [phase, blocksInThisPhase] : blocksByPhase) {
            if(phase + 1 == height) { //Last phase
                break;
            }

            auto blocksInNextPhase = blocksByPhase[phase + 1];

            for(int blockNumberThisPhase = 0; blockNumberThisPhase < blocksInThisPhase.size(); blockNumberThisPhase++){
                auto blockThisPhase = blocksInThisPhase.at(blockNumberThisPhase);

                auto childLeftOfBlockThisPhase = blocksInNextPhase.at(blockNumberThisPhase * 2);
                auto childRightOfBlockThisPhase = blocksInNextPhase.at((blockNumberThisPhase * 2) + 1);

                blockThisPhase->left = childLeftOfBlockThisPhase;
                blockThisPhase->right = childRightOfBlockThisPhase;
            }
        }

        return blocksByPhase;
    }
};