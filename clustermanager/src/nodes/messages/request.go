package messages

import (
	"encoding/binary"
	"strconv"
)

type Request struct {
	authKey        string
	operatorNumber uint8
	args           []byte
}

func BuildHealthCheckRequest(authKey string) Request {
	return Request{
		authKey:        authKey,
		operatorNumber: 0x04,
	}
}

func BuildGetClusterConfigRequest(authKey string) Request {
	return Request{
		authKey:        authKey,
		operatorNumber: 12,
	}
}

func BuildDoLeaveNodeClusterRequest(authKey string, nodeId string) Request {
	nodeIdU32, _ := strconv.Atoi(nodeId)
	nodeIdU16 := uint16(nodeIdU32)
	args := make([]byte, 0)
	args = binary.BigEndian.AppendUint32(args, 2)
	args = binary.BigEndian.AppendUint16(args, nodeIdU16)

	return Request{
		authKey:        authKey,
		operatorNumber: 15,
		args:           args,
	}
}

func BuildGetNodeDataRequest(authKey string, nodeId string) Request {
	nodeIdU32, _ := strconv.Atoi(nodeId)
	nodeIdU16 := uint16(nodeIdU32)
	args := make([]byte, 0)
	args = binary.BigEndian.AppendUint32(args, 2)
	args = binary.BigEndian.AppendUint16(args, nodeIdU16)

	return Request{
		authKey:        authKey,
		operatorNumber: 16,
		args:           args,
	}
}

func (request *Request) GetTotalLength() uint32 {
	var length uint32 = 0
	length += 4                                //Req number
	length += 1 + uint32(len(request.authKey)) //Auth
	length += 1                                //Op number
	length += 8                                //Timestamp
	length += uint32(len(request.args))

	return length
}
