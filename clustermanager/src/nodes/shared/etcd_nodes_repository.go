package shared

import (
	"clustermanager/src/_shared/etcd"
	"errors"
	"fmt"
	"strings"
)

type EtcdNodeRepository struct {
	Client *etcd.EtcdClient[Node]
}

func (repository EtcdNodeRepository) FindAll() ([]Node, error) {
	return repository.Client.GetAll("/nodes", etcd.JSON)
}

func (repository EtcdNodeRepository) FindByAddress(address string) (Node, error) {
	allNodes, err := repository.FindAll()

	if err != nil || len(allNodes) == 0 {
		return Node{}, err
	}

	for _, node := range allNodes {
		if strings.Split(node.Address, ":")[0] == address {
			return node, nil
		}
	}

	return Node{}, errors.New("node not found")
}

func (repository EtcdNodeRepository) FindById(nodeId NodeId_t) (Node, error) {
	return repository.Client.Get("/nodes/"+fmt.Sprint(nodeId), etcd.JSON)
}

func (repository EtcdNodeRepository) Add(node Node) error {
	return repository.Client.Put("/nodes/"+fmt.Sprint(node.NodeId), node, etcd.JSON)
}
