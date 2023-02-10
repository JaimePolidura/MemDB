package nodes

import (
	"clustermanager/src/_shared/etcd"
	"fmt"
)

type EtcdNodeRepository struct {
	Client *etcd.EtcdClient[Node]
}

func (repository EtcdNodeRepository) FindAll() ([]Node, error) {
	return repository.Client.GetAll("/nodes")
}

func (repository EtcdNodeRepository) FindById(nodeId uint32) (Node, error) {
	return repository.Client.Get("/nodes/" + fmt.Sprint(nodeId))
}

func (repository EtcdNodeRepository) Add(node Node) error {
	return repository.Client.Put("/nodes/"+fmt.Sprint(node.NodeId), node)
}
