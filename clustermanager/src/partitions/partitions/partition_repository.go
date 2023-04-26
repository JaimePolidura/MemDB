package partitions

type PartitionRepository interface {
	SaveRingEntries(newEntries []PartitionRingEntry) error
	GetRingEntries() ([]PartitionRingEntry, error)
	GetPartitionsByKey() (uint32, error)
	GetRingMaxSize() (uint32, error)
}
