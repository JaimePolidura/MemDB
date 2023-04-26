package shared

type PartitionRepository interface {
	GetPartitionByKey() (int, error)
	GetRing() (PartitionRing, error)
}
