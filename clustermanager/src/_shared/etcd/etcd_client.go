package etcd

import (
	"context"
	"encoding/json"
	"fmt"
	clientv3 "go.etcd.io/etcd/client/v3"
	"reflect"
	"time"
)

type EtcdClient[T any] struct {
	NativeClient *clientv3.Client
	Timeout      time.Duration
}

func (client EtcdClient[T]) Put(key string, value T, mediaType EtcdMediaType) error {
	valueToWriteEtcd, err := client.convertFromMediaType(value, mediaType)
	if err != nil {
		return err
	}

	ctx, cancel := context.WithTimeout(context.Background(), time.Second*5)
	_, err = client.NativeClient.Put(ctx, key, string(valueToWriteEtcd))
	cancel()

	return err //It will be nil if it success
}

func (client EtcdClient[T]) GetAll(key string, mediaType EtcdMediaType) ([]T, error) {
	ctx, cancel := context.WithTimeout(context.Background(), time.Second*5)
	response, err := client.NativeClient.Get(ctx, key, clientv3.WithPrefix())
	cancel()

	if err != nil {
		return nil, err
	}

	nodes := make([]T, response.Count)

	for index, value := range response.Kvs {
		node, err := client.convertToMediaType(value.Value, mediaType)
		if err != nil {
			return nil, err
		}

		nodes[index] = node
	}

	return nodes, nil
}

func (client EtcdClient[T]) Get(key string, mediaType EtcdMediaType) (T, error) {
	ctx, cancel := context.WithTimeout(context.Background(), client.Timeout)
	rawResponse, err := client.NativeClient.Get(ctx, key, clientv3.WithPrefix())
	cancel()

	if err != nil || len(rawResponse.Kvs) == 0 {
		var returnNull T
		return returnNull, err
	}

	return client.convertToMediaType(rawResponse.Kvs[0].Value, mediaType)
}

func (client EtcdClient[T]) convertToMediaType(value []byte, mediaType EtcdMediaType) (T, error) {
	if mediaType == JSON {
		var object T
		err := json.Unmarshal(value, &object)
		return object, err
	} else if mediaType == STRING {
		strValue := string(value)
		t := reflect.TypeOf("")
		v := reflect.New(t).Interface()
		reflect.ValueOf(v).Elem().SetString(strValue)
		return v.(T), nil
	}

	panic("Invalid media type")
}

func (client EtcdClient[T]) convertFromMediaType(value T, mediaType EtcdMediaType) (string, error) {
	if mediaType == JSON {
		valueBytes, err := json.Marshal(value)

		if err != nil {
			return "", err
		} else {
			return string(valueBytes), err
		}
	} else if mediaType == STRING {
		return fmt.Sprintf("%v", value), nil
	}

	panic("Invalid media type")
}

type EtcdMediaType string

const (
	JSON   EtcdMediaType = "JSON"
	STRING EtcdMediaType = "STRING"
)
