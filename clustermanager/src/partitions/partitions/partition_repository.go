package partitions

type PartitionRepository interface {
	Add(newEntry PartitionRingEntry) error
	GetRingEntries() (PartitionRingEntries, error)
	GetPartitionsByKey() (uint32, error)
	GetRingMaxSize() (uint32, error)
}
