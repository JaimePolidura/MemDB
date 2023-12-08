package messages

import (
	"encoding/binary"
)

func Desrialize(rawResponse []byte) Response {
	resultByte := rawResponse[14]
	success := (resultByte & 0x01) == 1
	errorCode := resultByte >> 1
	responseBody := getResponseBodyString(rawResponse)

	return Response{ErrorCode: errorCode, Success: success, ResponseBody: responseBody}
}

func getResponseBodyString(rawResponse []byte) string {
	responseLengthRaw := rawResponse[15:19] //Inclusive - exclusive
	ressponseLength := binary.BigEndian.Uint32(responseLengthRaw)

	if ressponseLength > 0 {
		return string(rawResponse[19:])
	} else {
		return ""
	}
}
