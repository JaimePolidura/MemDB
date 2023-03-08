#pragma once

#include <vector>
#include <thread>
#include <math.h>
#include <map>

#include "messages/request/Request.h"
#include "CompactionBlock.h"

class MultithreadedLogCompactor {
public:
    static std::vector<OperationBody> compact(allOperationLogs_t uncompacted) {
        return createCompactionTree(uncompacted)->get();
    }

private:
    static CompactionBlock * createCompactionTree(allOperationLogs_t uncompacted) {
        auto blocksByPhase = createBlocksByPhaseMap(uncompacted);
        blocksByPhase = fillTreeChilds(blocksByPhase);

        return blocksByPhase[0].at(0);
    }

    static std::map<int, std::vector<CompactionBlock *>> fillTreeChilds(std::map<int, std::vector<CompactionBlock *>>& blocksByPhase) {
        uint32_t numberThreads = std::thread::hardware_concurrency();
        uint32_t height = std::log2(numberThreads);

        for (const auto& [phase, blocksInThisPhase] : blocksByPhase) {
            if(phase + 1 == height) { //Last phase
                break;
            }

            int totalChildsInSubphase = std::pow(2, phase + 1);
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

    static std::map<int, std::vector<CompactionBlock *>> createBlocksByPhaseMap(allOperationLogs_t uncompacted) {
        uint32_t numberThreads = std::thread::hardware_concurrency();
        uint32_t height = std::log2(numberThreads);
        std::map<int, std::vector<CompactionBlock *>> blocksByPhase;

        for (int actualPhase = 0; actualPhase < height; actualPhase++) {
            blocksByPhase[actualPhase] = std::vector<CompactionBlock * >{};
            int nodesToCreateInThisPhase = std::pow(2, actualPhase + 1);
            bool firstPhase = actualPhase == 0;
            bool lastPhase = actualPhase + 1 == height;

            for(int j = 0; j < nodesToCreateInThisPhase; j++){
                if(firstPhase){
                    blocksByPhase[actualPhase].push_back(CompactionBlock::root());
                }else if(lastPhase) {
                    blocksByPhase[actualPhase].push_back(CompactionBlock::leaf(uncompacted, j, nodesToCreateInThisPhase));
                }else {
                    blocksByPhase[actualPhase].push_back(CompactionBlock::node());
                }
            }
        }

        return blocksByPhase;
    }
};