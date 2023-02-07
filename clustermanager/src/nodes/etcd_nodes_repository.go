package nodes

import clientv3 "go.etcd.io/etcd/client/v3"

type EtcdNodeRepository struct {
	client clientv3.Client
}

func (repository EtcdNodeRepository) findAll() []Node {
	return nil
}

func (repository EtcdNodeRepository) findById(nodeId uint32) (node Node, err error) {
	return node, nil
}

func (repository EtcdNodeRepository) add(node Node) error {
	return nil
}
