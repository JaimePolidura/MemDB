package partitions

import (
	"clustermanager/src/_shared/etcd"
	"encoding/json"
	"sort"
	"strconv"
)

type EtcdPartitionRepository struct {
	Client *etcd.EtcdClient[string]
}

func (repository EtcdPartitionRepository) Add(newEntriy PartitionRingEntry) error {
	entriesByte, err := json.Marshal(newEntriy)
	if err != nil {
		return err
	}

	return repository.Client.Put("/partitions/ring/"+strconv.Itoa(int(newEntriy.RingPosition)), string(entriesByte), etcd.STRING)
}

func (repository EtcdPartitionRepository) GetRingMaxSize() (uint32, error) {
	valueStr, err := repository.Client.Get("/partitions/config/ringSize", etcd.STRING)
	valueInt, err := strconv.Atoi(valueStr)

	if err != nil {
		return 0, err
	}

	return uint32(valueInt), nil
}

func (repository EtcdPartitionRepository) GetNodesPerPartition() (uint32, error) {
	valueStr, err := repository.Client.Get("/partitions/config/nodesPerPartition", etcd.STRING)
	valueInt, err := strconv.Atoi(valueStr)

	if err != nil {
		return 0, err
	}

	return uint32(valueInt), nil
}

func (repository EtcdPartitionRepository) GetRingEntriesSorted() (PartitionRingEntries, error) {
	valueStrJson, err := repository.Client.Get("/partitions/ring", etcd.STRING)

	if err != nil {
		return PartitionRingEntries{}, err
	}

	var partitionRing []PartitionRingEntry
	err = json.Unmarshal([]byte(valueStrJson), &partitionRing)

	sort.Slice(partitionRing, func(i, j int) bool {
		return partitionRing[i].RingPosition < partitionRing[j].RingPosition
	})

	return PartitionRingEntries{Entries: partitionRing}, err
}
