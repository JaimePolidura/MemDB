package nodes

import (
	"clustermanager/src/nodes/messages"
	"encoding/binary"
	"errors"
	"fmt"
	"net"
)

type NodeId_t string

type Node struct {
	NodeId     NodeId_t
	Address    string
	connection net.Conn
}

func (node *Node) Send(requestToSend messages.Request) (messages.Response, error) {
	if err := node.openConnectionIfClosed(); err != nil {
		return messages.Response{}, err
	}
	if _, err := node.connection.Write(messages.SerializeRequest(requestToSend)); err != nil {
		return messages.Response{}, err
	}

	if bytes, err := node.readResponseBytes(); err == nil {
		fmt.Println(bytes)

		return messages.Desrialize(bytes), nil
	} else {
		return messages.Response{}, err
	}
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

func (node *Node) readResponseBytes() ([]byte, error) {
	fragHeader := make([]byte, 1)
	_, err := node.connection.Read(fragHeader)
	if err != nil {
		return nil, err
	} else if fragHeader[0] != 0 {
		return nil, errors.New("fragmentation packets no implemented")
	}

	responseLengthBytes := make([]byte, 4)
	_, err = node.connection.Read(responseLengthBytes)
	if err != nil {
		return nil, err
	}

	responseBytes := make([]byte, binary.BigEndian.Uint32(responseLengthBytes))
	_, err = node.connection.Read(responseBytes)
	if err != nil {
		return nil, err
	}

	return responseBytes, nil
}
