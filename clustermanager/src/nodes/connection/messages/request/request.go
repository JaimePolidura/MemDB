package request

type Request struct {
	authKey        string
	operatorNumber uint8
	args           []string
}
