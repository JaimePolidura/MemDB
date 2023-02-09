package request

import "fmt"

func SerializeRequest(request *Request) []byte {
	bytes := make([]byte, 0)

	fmt.Println(bytes)

	bytes = append(bytes, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)
	fmt.Println(bytes)

	bytes = append(bytes, byte(len(request.authKey)<<2))
	fmt.Println(bytes)

	bytes = append(bytes, []byte(request.authKey)...)
	fmt.Println(bytes)

	bytesx = append(bytes, request.operatorNumber<<2)
	bytes = append(bytes, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)
	fmt.Println(bytes)

	for _, arg := range request.args {
		bytes = append(bytes, byte(len(arg)))
		bytes = append(bytes, []byte(arg)...)
	}

	fmt.Println(bytes)

	return bytes
}
