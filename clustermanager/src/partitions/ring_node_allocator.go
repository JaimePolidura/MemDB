package partitions

import (
	"clustermanager/src/_shared/utils"
	"clustermanager/src/nodes/nodes"
	"clustermanager/src/partitions/partitions"
	"errors"
	"math/rand"
)

type RingNodeAllocator struct {
	HashCalculator       utils.HashCalculator
	PartitionsRepository partitions.PartitionRepository
	NodeRepository       nodes.NodeRepository
}

func (ringNodeAllocator *RingNodeAllocator) CanAllocateNode(nodeId nodes.NodeId_t) bool {
	ringEntriesSorted, err := ringNodeAllocator.PartitionsRepository.GetRingEntriesSorted()
	nodesPerPartition, err := ringNodeAllocator.PartitionsRepository.GetNodesPerPartition()
	if err != nil {
		return false
	}

	neighbors, _ := ringEntriesSorted.GetNeighborsByNodeId(nodeId, nodesPerPartition)

	for _, neighbor := range neighbors {
		node, err := ringNodeAllocator.NodeRepository.FindById(neighbor.NodeId)
		if err != nil && node.State != nodes.RUNNING {
			return false
		}
	}

	return true
}

func (ringNodeAllocator *RingNodeAllocator) Allocate(nodeId nodes.NodeId_t) (partitions.PartitionRingEntry, error) {
	positionInRing, err := ringNodeAllocator.getPositionRingOfNode(nodeId)
	if err != nil {
		return partitions.PartitionRingEntry{}, err
	}

	newEntry := partitions.PartitionRingEntry{NodeId: nodeId, RingPosition: positionInRing}
	ringNodeAllocator.PartitionsRepository.Add(newEntry)

	return newEntry, nil
}

func (ringNodeAllocator *RingNodeAllocator) getPositionRingOfNode(nodeId nodes.NodeId_t) (uint32, error) {
	ringEntries, err := ringNodeAllocator.PartitionsRepository.GetRingEntriesSorted()
	hashValue := ringNodeAllocator.HashCalculator.Calculate(string(nodeId))

	ringSize, err := ringNodeAllocator.PartitionsRepository.GetRingMaxSize()
	positionInRing := uint32(hashValue) % ringSize
	if err != nil {
		return 0, err
	}
	attempts := 100

	for {
		if attempts == 0 {
			return 0, errors.New("cannot select node")
		}

		if positionAlreadyOccupied(ringEntries.Entries, positionInRing) || ringNodeAllocator.nodeNotAvailable(ringEntries, positionInRing) {
			positionInRing = ringNodeAllocator.getRandomNumber(positionInRing)
			attempts--
		} else {
			break
		}
	}

	return positionInRing, nil
}

func (ringNodeAllocator *RingNodeAllocator) nodeNotAvailable(ringEntries partitions.PartitionRingEntries, position uint32) bool {
	_, counterClockWiseNeighbor, _ := ringEntries.GetNeighborByRingPosition(position)
	nodeClockwise, err := ringNodeAllocator.NodeRepository.FindById(counterClockWiseNeighbor.NodeId)

	if err != nil {
		return true
	}

	return nodeClockwise.State != nodes.RUNNING
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
