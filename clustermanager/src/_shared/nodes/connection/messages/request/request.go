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
