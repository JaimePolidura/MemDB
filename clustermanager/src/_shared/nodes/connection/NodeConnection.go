package connection

import (
	"clustermanager/src/_shared/nodes"
	request2 "clustermanager/src/_shared/nodes/connection/messages/request"
	response2 "clustermanager/src/_shared/nodes/connection/messages/response"
	"net"
)

type NodeConnection struct {
	connection net.Conn
	node       nodes.Node
}

func (nodeConnection *NodeConnection) Send(requestToSend request2.Request) (response2.Response, error) {
	_, err := nodeConnection.connection.Write(request2.SerializeRequest(requestToSend))

	if err != nil {
		return response2.Response{}, err
	}

	responseBuffer := make([]byte, 274)
	_, err = nodeConnection.connection.Read(responseBuffer)

	if err != nil {
		return response2.Response{}, err
	}

	return response2.Desrialize(responseBuffer), nil
}
