package connection

import (
	"clustermanager/src/nodes"
	"net"
)

type NodeConnections struct {
	connections map[uint32]NodeConnection
}

func (nodeConnections *NodeConnections) GetByIdOrCreate(node nodes.Node) (NodeConnection, error) {
	connection, exists := nodeConnections.GetByNodeId(node.NodeId)

	if exists {
		return connection, nil
	} else {
		return nodeConnections.Create(node)
	}
}

func (nodeConnections *NodeConnections) Create(node nodes.Node) (NodeConnection, error) {
	tcpConnection, err := net.Dial("tcp", node.Address)
	if err != nil {
		return NodeConnection{}, err
	}

	connection := NodeConnection{connection: tcpConnection, node: node}
	nodeConnections.connections[node.NodeId] = connection

	return connection, nil
}

func (nodeConnections *NodeConnections) GetByNodeId(nodeId uint32) (NodeConnection, bool) {
	value, exists := nodeConnections.connections[nodeId]
	return value, exists
}
