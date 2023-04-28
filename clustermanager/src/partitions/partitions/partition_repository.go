package partitions

type PartitionRepository interface {
	Add(newEntry PartitionRingEntry) error
	GetRingEntriesSorted() (PartitionRingEntries, error)
	GetNodesPerPartition() (uint32, error)
	GetRingMaxSize() (uint32, error)
}
