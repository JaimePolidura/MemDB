package partitions

import (
	"clustermanager/src/_shared/etcd"
	"encoding/json"
	"sort"
	"strconv"
	"sync"
)

type PartitionRepository struct {
	Client etcd.EtcdClient[string]

	partitionRingEntriesCache     PartitionRingEntries
	partitionRingEntriesCacheLock sync.Mutex
}

func (repository *PartitionRepository) Add(newEntriy PartitionRingEntry) error {
	entriesByte, err := json.Marshal(newEntriy)
	if err != nil {
		return err
	}

	repository.partitionRingEntriesCacheLock.Lock()
	repository.partitionRingEntriesCache.Add(newEntriy)
	repository.partitionRingEntriesCacheLock.Unlock()

	return repository.Client.Put("/partitions/ring/"+strconv.Itoa(int(newEntriy.RingPosition)), string(entriesByte), etcd.STRING)
}

func (repository *PartitionRepository) GetRingMaxSize() (uint32, error) {
	valueStr, err := repository.Client.Get("/partitions/config/ringSize", etcd.STRING)
	valueInt, err := strconv.Atoi(valueStr)

	if err != nil {
		return 0, err
	}

	return uint32(valueInt), nil
}

func (repository *PartitionRepository) GetNodesPerPartition() (uint32, error) {
	valueStr, err := repository.Client.Get("/partitions/config/nodesPerPartition", etcd.STRING)
	valueInt, err := strconv.Atoi(valueStr)

	if err != nil {
		return 0, err
	}

	return uint32(valueInt), nil
}

func (repository *PartitionRepository) GetRingEntriesSorted() (PartitionRingEntries, error) {
	if len(repository.partitionRingEntriesCache.Entries) > 0 {
		return repository.partitionRingEntriesCache, nil
	}

	valueStrJson, err := repository.Client.Get("/partitions/ring", etcd.STRING) //error

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
