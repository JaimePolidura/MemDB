package response

import (
	"encoding/binary"
)

func Desrialize(rawResponse []byte) Response {
	resultByte := rawResponse[16]
	success := (resultByte & 0x01) == 1
	errorCode := resultByte >> 1
	responseBody := getResponseBodyString(rawResponse)

	return Response{ErrorCode: errorCode, Success: success, ResponseBody: responseBody}
}

func getResponseBodyString(rawResponse []byte) string {
	var responseBody string

	if len(rawResponse) > 17 {
		ressponseLength := binary.BigEndian.Uint32(rawResponse[13:16])
		responseBody = string(rawResponse[17:(17 + ressponseLength)])
	} else {
		responseBody = ""
	}

	return responseBody
}
