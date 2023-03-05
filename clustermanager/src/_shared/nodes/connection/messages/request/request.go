package request

type Request struct {
	authKey        string
	operatorNumber uint8
	args           []string
}

func BuildHealthCheckRequest(authKey string) Request {
	return Request{
		authKey:        authKey,
		operatorNumber: 0x04,
	}
}

func (request *Request) GetTotalLength() uint32 {
	var length uint32 = 0
	length += 4                                //Req number
	length += 1 + uint32(len(request.authKey)) //Auth
	length += 1                                //Op number
	length += 8                                //Timestamp

	for i := 0; i < len(request.args); i++ {
		length += 4                            //Arg size
		length += uint32(len(request.args[i])) //Arg size
	}

	return length
}
