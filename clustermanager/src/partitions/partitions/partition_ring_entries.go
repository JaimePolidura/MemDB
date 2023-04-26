package partitions

import (
	"clustermanager/src/_shared/utils"
	"clustermanager/src/nodes/nodes"
)

type PartitionRingEntries struct {
	Entries []PartitionRingEntry
}

type PartitionRingEntry struct {
	NodeId       nodes.NodeId_t `json:"nodeId"`
	RingPosition uint32         `json:"ringPosition"`
}

func (entries PartitionRingEntries) GetNeighborByRingPosition(ringPosition uint32) (PartitionRingEntry, PartitionRingEntry, bool) { //Clock wise, counter clock wise, found
	clockWise, counterClockWise, found := entries.GetNeighborsByRingPosition(ringPosition, 1)
	if !found {
		return PartitionRingEntry{}, PartitionRingEntry{}, found
	}
	
	return clockWise[0], counterClockWise[0], true
}

func (entries PartitionRingEntries) GetNeighborsByRingPosition(ringPosition uint32, numberNeighbors int) ([]PartitionRingEntry, []PartitionRingEntry, bool) { //Clock wise, counter clock wise, found
	indexOfRingEntry, found := entries.getIndexByRingPosition(ringPosition)
	if !found {
		return []PartitionRingEntry{}, []PartitionRingEntry{}, false
	}

	ringIterator := utils.CircularArrayIterator[PartitionRingEntry]{Array: entries.Entries}
	ringIterator.WithStartingIndex(indexOfRingEntry)
	var counterClockWise []PartitionRingEntry
	var clockWise []PartitionRingEntry

	for i := 0; i < numberNeighbors; i++ {
		counterClockWise = append(counterClockWise, ringIterator.Back())
		clockWise = append(clockWise, ringIterator.Next())
	}

	return clockWise, counterClockWise, true
}

func (entries PartitionRingEntries) getClockWiseInRing(index int) PartitionRingEntry {
	if index == len(entries.Entries)-1 {
		return entries.Entries[0]
	} else {
		return entries.Entries[index+1]
	}
}

func (entries PartitionRingEntries) getCounterClockWiseInRing(index int) PartitionRingEntry {
	if index == 0 {
		return entries.Entries[len(entries.Entries)-1]
	} else {
		return entries.Entries[index-1]
	}
}

func (entries PartitionRingEntries) getIndexByRingPosition(ringPositionToLook uint32) (int, bool) {
	for index, entry := range entries.Entries {
		if entry.RingPosition == ringPositionToLook {
			return index, true
		}
	}

	return -1, false
}
