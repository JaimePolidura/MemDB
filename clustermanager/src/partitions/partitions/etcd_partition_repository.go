package partitions

import (
	"clustermanager/src/_shared/etcd"
	"encoding/json"
	"strconv"
)

type EtcdPartitionRepository struct {
	Client *etcd.EtcdClient[string]
}

func (repository EtcdPartitionRepository) SaveRingEntries(newEntries []PartitionRingEntry) error {
	entriesByte, err := json.Marshal(newEntries)
	if err != nil {
		return err
	}

	return repository.Client.Put("/partitions/ring", string(entriesByte), etcd.STRING)
}

func (repository EtcdPartitionRepository) GetRingMaxSize() (uint32, error) {
	valueStr, err := repository.Client.Get("/partitions/config/ringSize", etcd.STRING)
	valueInt, err := strconv.Atoi(valueStr)

	if err != nil {
		return 0, err
	}

	return uint32(valueInt), nil
}

func (repository EtcdPartitionRepository) GetPartitionsByKey() (uint32, error) {
	valueStr, err := repository.Client.Get("/partitions/config/partitionsPerKey", etcd.STRING)
	valueInt, err := strconv.Atoi(valueStr)

	if err != nil {
		return 0, err
	}

	return uint32(valueInt), nil
}

func (repository EtcdPartitionRepository) GetRingEntries() ([]PartitionRingEntry, error) {
	valueStrJson, err := repository.Client.Get("/partitions/ring", etcd.STRING)

	if err != nil {
		return []PartitionRingEntry{}, err
	}

	var partitionRing []PartitionRingEntry
	err = json.Unmarshal([]byte(valueStrJson), &partitionRing)

	return partitionRing, err
}
