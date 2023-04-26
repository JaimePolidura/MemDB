package partitions

type PartitionRepository interface {
	Add(newEntry PartitionRingEntry) error
	GetRingEntries() ([]PartitionRingEntry, error)
	GetPartitionsByKey() (uint32, error)
	GetRingMaxSize() (uint32, error)
}
