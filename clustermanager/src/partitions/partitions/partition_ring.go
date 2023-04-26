package partitions

type PartitionRing struct {
	entries []PartitionRingEntry
}

type PartitionRingEntry struct {
	ringPosition int
	nodeId       int
}
