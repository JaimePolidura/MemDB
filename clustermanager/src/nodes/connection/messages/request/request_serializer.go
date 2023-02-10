package request

func SerializeRequest(request *Request) []byte {
	bytes := make([]byte, 0)

	bytes = append(bytes, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)
	bytes = append(bytes, byte(len(request.authKey)<<2))
	bytes = append(bytes, []byte(request.authKey)...)
	bytes = append(bytes, request.operatorNumber<<2)
	bytes = append(bytes, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)

	for _, arg := range request.args {
		bytes = append(bytes, byte(len(arg)))
		bytes = append(bytes, []byte(arg)...)
	}

	return bytes
}
