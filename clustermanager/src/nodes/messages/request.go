package messages

type Request struct {
	AuthKey        string
	OperatorNumber uint8
	Args           []string
}

func BuildHealthCheckRequest(authKey string) Request {
	return Request{
		AuthKey:        authKey,
		OperatorNumber: 0x04,
	}
}

func BuildGetClusterConfigRequest(authKey string) Request {
	return Request{
		AuthKey:        authKey,
		OperatorNumber: 0x12,
	}
}

func BuildDoLeaveNodeClusterRequest(authKey string, nodeId string) Request {
	return Request{
		AuthKey:        authKey,
		OperatorNumber: 0x15,
		Args:           []string{nodeId},
	}
}

func BuildGetNodeDataRequest(authKey string, nodeId string) Request {
	return Request{
		AuthKey:        authKey,
		OperatorNumber: 0x16,
		Args:           []string{nodeId},
	}
}

func (request *Request) GetTotalLength() uint32 {
	var length uint32 = 0
	length += 4                                //Req number
	length += 1 + uint32(len(request.AuthKey)) //Auth
	length += 1                                //Op number
	length += 8                                //Timestamp

	for i := 0; i < len(request.Args); i++ {
		length += 4                            //Arg size
		length += uint32(len(request.Args[i])) //Arg size
	}

	return length
}
