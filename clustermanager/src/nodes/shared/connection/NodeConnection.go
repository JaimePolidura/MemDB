package connection

import (
	"clustermanager/src/nodes/shared"
	"clustermanager/src/nodes/shared/connection/messages/request"
	"clustermanager/src/nodes/shared/connection/messages/response"
	"net"
)

type NodeConnection struct {
	connection net.Conn
	node       shared.Node
}

func (nodeConnection *NodeConnection) Send(requestToSend request.Request) (response.Response, error) {
	_, err := nodeConnection.connection.Write(request.SerializeRequest(requestToSend))

	if err != nil {
		return response.Response{}, err
	}

	responseBuffer := make([]byte, 274)
	_, err = nodeConnection.connection.Read(responseBuffer)

	if err != nil {
		return response.Response{}, err
	}

	return response.Desrialize(responseBuffer), nil
}
