package connection

import (
	"clustermanager/src/_shared/logging"
	"clustermanager/src/_shared/nodes"
	"net"
	"sync"
)

type NodeConnections struct {
	connections sync.Map
	logger      *logging.Logger
}

func (nodeConnections *NodeConnections) GetByIdOrCreate(node nodes.Node) (*NodeConnection, error) {
	connection, exists := nodeConnections.GetByNodeId(node.NodeId)

	if exists {
		return connection, nil
	} else {
		return nodeConnections.Create(node)
	}
}

func (nodeConnections *NodeConnections) Delete(nodeId nodes.NodeId_t) {
	nodeConnections.connections.Delete(nodeId)
}

func (nodeConnections *NodeConnections) Create(node nodes.Node) (*NodeConnection, error) {
	if conn, exists := nodeConnections.GetByNodeId(node.NodeId); exists {
		conn.connection.Close()
		nodeConnections.Delete(node.NodeId)
	}

	tcpConnection, err := net.DialTimeout("tcp", node.Address, 10)
	if err != nil {
		return &NodeConnection{}, err
	}

	connection := &NodeConnection{connection: tcpConnection, node: node}

	nodeConnections.connections.Store(node.NodeId, connection)

	return connection, nil
}

func (nodeConnections *NodeConnections) GetByNodeId(nodeId nodes.NodeId_t) (*NodeConnection, bool) {
	value, exists := nodeConnections.connections.Load(nodeId)

	if exists {
		return value.(*NodeConnection), true
	} else {
		return nil, false
	}

}

func CreateNodeConnectionsObject(logger *logging.Logger) *NodeConnections {
	return &NodeConnections{connections: sync.Map{}, logger: logger}
}
