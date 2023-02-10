package connection

import (
	"clustermanager/src/nodes"
	"net"
)

type NodeConnections struct {
	connections map[nodes.NodeId_t]NodeConnection
}

func (nodeConnections *NodeConnections) GetByIdOrCreate(node nodes.Node) (NodeConnection, error) {
	connection, exists := nodeConnections.GetByNodeId(node.NodeId)

	if exists {
		return connection, nil
	} else {
		return nodeConnections.Create(node)
	}
}

func (nodeConnections *NodeConnections) Delete(nodeId nodes.NodeId_t) {
	delete(nodeConnections.connections, nodeId)
}

func (nodeConnections *NodeConnections) Create(node nodes.Node) (NodeConnection, error) {
	tcpConnection, err := net.DialTimeout("tcp", node.Address, 10)
	if err != nil {
		return NodeConnection{}, err
	}

	connection := NodeConnection{connection: tcpConnection, node: node}
	nodeConnections.connections[node.NodeId] = connection

	return connection, nil
}

func (nodeConnections *NodeConnections) GetByNodeId(nodeId nodes.NodeId_t) (NodeConnection, bool) {
	value, exists := nodeConnections.connections[nodeId]
	return value, exists
}

func CreateNodeConnectionsObject() *NodeConnections {
	return &NodeConnections{connections: make(map[nodes.NodeId_t]NodeConnection)}
}
