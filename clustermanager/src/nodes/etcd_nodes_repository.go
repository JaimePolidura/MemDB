package nodes

import (
	"context"
	"encoding/json"
	"fmt"
	clientv3 "go.etcd.io/etcd/client/v3"
	"time"
)

type EtcdNodeRepository struct {
	client clientv3.Client
}

func (repository EtcdNodeRepository) FindAll() ([]Node, error) {
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*5)
	response, err := repository.client.Get(ctx, "/nodes/", clientv3.WithPrefix())
	cancel()
	if err != nil {
		return nil, err
	}

	nodes := make([]Node, response.Count)

	for index, value := range response.Kvs {
		var node Node
		err := json.Unmarshal(value.Value, &node)
		if err != nil {
			return nil, err
		}

		nodes[index] = node
	}

	return nodes, nil
}

func (repository EtcdNodeRepository) FindById(nodeId uint32) (Node, error) {
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*5)
	rawResponse, err := repository.client.Get(ctx, "/nodes/"+fmt.Sprint(nodeId), clientv3.WithPrefix())
	cancel()
	if err != nil {
		return Node{}, err
	}

	var nodeToReturn Node
	if err := json.Unmarshal(rawResponse.Kvs[0].Value, &nodeToReturn); err != nil {
		return nodeToReturn, err
	}

	return nodeToReturn, nil
}

func (repository EtcdNodeRepository) Add(node Node) error {
	nodeJson, err := json.Marshal(node)
	if err != nil {
		return err
	}

	ctx, cancel := context.WithTimeout(context.Background(), time.Second*5)
	_, err = repository.client.Put(ctx, "/nodes/"+fmt.Sprint(node.NodeId), string(nodeJson))
	cancel()

	return err //It might be nil
}
