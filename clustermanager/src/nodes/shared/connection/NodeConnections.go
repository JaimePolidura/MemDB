package connection

import (
	"clustermanager/src/_shared/logging"
	"clustermanager/src/_shared/utils"
	"clustermanager/src/nodes/shared"
	"net"
	"sync"
)

type NodeConnections struct {
	connections sync.Map
	logger      *logging.Logger
}

func (nodeConnections *NodeConnections) GetByIdOrCreate(node shared.Node) (*NodeConnection, error) {
	connection, exists := nodeConnections.GetByNodeId(node.NodeId)

	if exists {
		return connection, nil
	} else {
		return nodeConnections.Create(node)
	}
}

func (nodeConnections *NodeConnections) Delete(nodeId shared.NodeId_t) {
	connection, exist := nodeConnections.GetByNodeId(nodeId)

	if exist {
		connection.connection.Close()
		nodeConnections.connections.Delete(nodeId)
	}
}

func (nodeConnections *NodeConnections) Create(node shared.Node) (*NodeConnection, error) {
	if conn, exists := nodeConnections.GetByNodeId(node.NodeId); exists {
		conn.connection.Close()
		nodeConnections.Delete(node.NodeId)
	}

	connection, err := nodeConnections.createTCPConnection(node)
	if err != nil {
		return nil, err
	}

	nodeConnections.connections.Store(node.NodeId, connection)

	return connection, nil
}

func (nodeConnections *NodeConnections) GetByNodeId(nodeId shared.NodeId_t) (*NodeConnection, bool) {
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

func (nodeConnections *NodeConnections) createTCPConnection(node shared.Node) (*NodeConnection, error) {
	adr, err := utils.GetTCPAddress(node.Address)
	tcpConnection, err := net.DialTCP("tcp", nil, adr)

	if err != nil {
		return nil, err
	} else {
		return &NodeConnection{connection: tcpConnection, node: node}, err
	}
}
