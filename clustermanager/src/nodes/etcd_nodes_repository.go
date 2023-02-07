package nodes

import (
	"clustermanager/src/_shared/etcd"
	"fmt"
)

type EtcdNodeRepository struct {
	client etcd.EtcdClient[Node]
}

func (repository EtcdNodeRepository) FindAll() ([]Node, error) {
	return repository.client.GetAll("/nodes")
}

func (repository EtcdNodeRepository) FindById(nodeId uint32) (Node, error) {
	return repository.client.Get("nodes/" + fmt.Sprint(nodeId))
}

func (repository EtcdNodeRepository) Add(node Node) error {
	return repository.client.Put(fmt.Sprint(node.NodeId), node)
}
