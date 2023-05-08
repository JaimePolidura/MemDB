package nodes

import (
	"clustermanager/src/_shared/etcd"
	"errors"
	"fmt"
	"strings"
	"sync"
)

type NodeRepository struct {
	Client etcd.EtcdClient[Node]

	nodesByIdCache sync.Map
}

func (repository *NodeRepository) FindAll() ([]Node, error) {
	return repository.Client.GetAll("/nodes", etcd.JSON)
}

func (repository *NodeRepository) FindByAddress(address string) (Node, error) {
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

func (repository *NodeRepository) FindById(nodeId NodeId_t) (Node, error) {
	if node, found := repository.nodesByIdCache.Load(nodeId); found {
		return node.(Node), nil
	}

	node, err := repository.Client.Get("/nodes/"+fmt.Sprint(nodeId), etcd.JSON)

	if err != nil {
		return Node{}, err
	}

	repository.nodesByIdCache.Store(node.NodeId, node)

	return node, nil
}

func (repository *NodeRepository) Add(node Node) error {
	return repository.Client.Put("/nodes/"+fmt.Sprint(node.NodeId), node, etcd.JSON)
}
