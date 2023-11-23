package nodes

import (
	"clustermanager/src/nodes/messages/request"
	"clustermanager/src/nodes/messages/response"
	"net"
)

type NodeId_t string

type Node struct {
	NodeId     NodeId_t
	Address    string
	connection net.Conn
}

func (node *Node) Send(requestToSend request.Request) (response.Response, error) {
	if err := node.openConnectionIfClosed(); err != nil {
		return response.Response{}, err
	}
	if _, err := node.connection.Write(request.SerializeRequest(requestToSend)); err != nil {
		return response.Response{}, err
	}

	responseBuffer := make([]byte, 274)

	if _, err := node.connection.Read(responseBuffer); err != nil {
		return response.Response{}, err
	}

	return response.Desrialize(responseBuffer), nil
}

func (node *Node) openConnectionIfClosed() error {
	if node.connection == nil {
		connection, err := net.Dial("tcp", node.Address)
		node.connection = connection
		return err
	} else {
		return nil
	}
}
