package partitions

import (
	"clustermanager/src/_shared/utils"
	"clustermanager/src/nodes/nodes"
	"clustermanager/src/partitions/partitions"
	"math/rand"
)

type RingNodeAllocator struct {
	HashCalculator       utils.HashCalculator
	PartitionsRepository partitions.PartitionRepository
}

func (ringNodeAllocator *RingNodeAllocator) Allocate(nodeId nodes.NodeId_t) (partitions.PartitionRingEntry, error) {
	ringEntries, err := ringNodeAllocator.PartitionsRepository.GetRingEntries()
	positionInRing, err := ringNodeAllocator.getPositionRingOfNode(nodeId, ringEntries)
	if err != nil {
		return partitions.PartitionRingEntry{}, err
	}

	newEntry := partitions.PartitionRingEntry{NodeId: nodeId, RingPosition: positionInRing}
	ringEntries = append(ringEntries, newEntry)

	ringNodeAllocator.PartitionsRepository.SaveRingEntries(ringEntries)

	return newEntry, nil
}

func (ringNodeAllocator *RingNodeAllocator) getPositionRingOfNode(nodeId nodes.NodeId_t, ringEntries []partitions.PartitionRingEntry) (uint32, error) {
	hashValue := ringNodeAllocator.HashCalculator.Calculate(string(nodeId))

	ringSize, err := ringNodeAllocator.PartitionsRepository.GetRingMaxSize()
	positionInRing := uint32(hashValue) % ringSize
	if err != nil {
		return 0, err
	}

	for {
		if positionAlreadyOccupied(ringEntries, positionInRing) {
			positionInRing = ringNodeAllocator.getRandomNumber(positionInRing)
		} else {
			break
		}
	}

	return positionInRing, nil
}

func positionAlreadyOccupied(ringEntries []partitions.PartitionRingEntry, position uint32) bool {
	for _, entry := range ringEntries {
		if entry.RingPosition == position {
			return true
		}
	}

	return false
}

func (ringNodeAllocator *RingNodeAllocator) getRandomNumber(maxRingSize uint32) uint32 {
	return rand.Uint32() % maxRingSize
}
