package partitions

import (
	"clustermanager/src/_shared/etcd"
	"encoding/json"
	"strconv"
)

type EtcdPartitionRepository struct {
	Client *etcd.EtcdClient[string]
}

func (repository EtcdPartitionRepository) GetPartitionByKey() (int, error) {
	valueStr, err := repository.Client.Get("/partitions/config/partitionsPerKey", etcd.STRING)

	if err != nil {
		return -1, err
	}

	return strconv.Atoi(valueStr)
}

func (repository EtcdPartitionRepository) GetRing() (PartitionRing, error) {
	valueStrJson, err := repository.Client.Get("/partitions/config/ring", etcd.STRING)

	if err != nil {
		return PartitionRing{}, err
	}

	var partitionRing PartitionRing
	err = json.Unmarshal([]byte(valueStrJson), &partitionRing)

	return partitionRing, err
}
