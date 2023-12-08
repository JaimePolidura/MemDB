package nodes

import (
	configuration "clustermanager/src/config"
	"clustermanager/src/nodes/messages"
	"encoding/binary"
	"errors"
	"strconv"
	"sync"
)

type ClusterNodeConnections struct {
	configuration *configuration.Configuartion

	nodesById *sync.Map
	seedNodes []*Node
}

type NodeData struct {
	nodeId  NodeId_t
	address string
}

func CreateClusterNodeConnections(configuartion *configuration.Configuartion) *ClusterNodeConnections {
	connections := &ClusterNodeConnections{nodesById: &sync.Map{}, configuration: configuartion}

	for _, seedNodeAddress := range connections.configuration.GetConfig(configuration.SEED_NODES) {
		node := &Node{NodeId: "0", Address: seedNodeAddress}
		connections.seedNodes = append(connections.seedNodes, node)
	}

	return connections
}

func (this *ClusterNodeConnections) GetNode(nodeId NodeId_t) (*Node, error) {
	if node, contained := this.nodesById.Load(nodeId); contained {
		return node.(*Node), nil
	}

	authKey := this.configuration.Get(configuration.AUTH_API_KEY)
	res, err := this.SendRequestToAnySeeder(messages.BuildGetNodeDataRequest(authKey, string(nodeId)))
	if err != nil || !res.Success {
		return nil, err
	}

	nodeDataResponse := getNodeDataResFromStirng(res)
	node := &Node{NodeId: nodeDataResponse.nodeId, Address: nodeDataResponse.address}

	this.nodesById.Store(node.NodeId, node)

	return node, nil
}

func (this *ClusterNodeConnections) SendRequestToAnySeeder(request messages.Request) (messages.Response, error) {
	for _, node := range this.seedNodes {
		if res, err := node.Send(request); err == nil {
			return res, nil
		}
	}

	return messages.Response{}, errors.New("No seed node found")
}

func getNodeDataResFromStirng(res messages.Response) NodeData {
	bytes := []byte(res.ResponseBody)
	nodeId := NodeId_t(strconv.Itoa(int(binary.BigEndian.Uint32(bytes[:4]))))
	addressSize := binary.BigEndian.Uint32(bytes[4:8])
	address := string(bytes[8:addressSize])

	return NodeData{
		nodeId:  nodeId,
		address: address,
	}
}
