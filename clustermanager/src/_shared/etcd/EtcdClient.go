package etcd

import (
	"context"
	"encoding/json"
	clientv3 "go.etcd.io/etcd/client/v3"
	"time"
)

type EtcdClient[T any] struct {
	NativeClient *clientv3.Client
	Timeout      time.Duration
}

func (client EtcdClient[T]) Put(key string, value T) error {
	valueJson, err := json.Marshal(value)
	if err != nil {
		return err
	}

	ctx, cancel := context.WithTimeout(context.Background(), time.Second*5)
	_, err = client.NativeClient.Put(ctx, key, string(valueJson))
	cancel()

	return err //It will be nil if it success
}

func (client EtcdClient[T]) GetAll(key string) ([]T, error) {
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*5)
	response, err := client.NativeClient.Get(ctx, key, clientv3.WithPrefix())
	cancel()

	if err != nil {
		return nil, err
	}

	nodes := make([]T, response.Count)

	for index, value := range response.Kvs {
		var node T
		err := json.Unmarshal(value.Value, &node)
		if err != nil {
			return nil, err
		}

		nodes[index] = node
	}

	return nodes, nil
}

func (client EtcdClient[T]) Get(key string) (T, error) {
	ctx, cancel := context.WithTimeout(context.Background(), client.Timeout)
	rawResponse, err := client.NativeClient.Get(ctx, key, clientv3.WithPrefix())
	cancel()

	if err != nil {
		var returnNull T
		return returnNull, err
	}

	var nodeToReturn T
	if err := json.Unmarshal(rawResponse.Kvs[0].Value, &nodeToReturn); err != nil {
		return nodeToReturn, err
	}

	return nodeToReturn, nil
}
