package response

func Desrialize(rawResponse []byte) Response {
	resultByte := rawResponse[16]
	success := (resultByte & 0x01) == 1
	errorCode := uint8(resultByte >> 1)
	responseBody := getResponseBodyString(rawResponse)

	return Response{errorCode: errorCode, success: success, responseBody: responseBody}
}

func getResponseBodyString(rawResponse []byte) string {
	var responseBody string

	if len(rawResponse) > 18 {
		ressponseLength := uint8(rawResponse[17])
		responseBody = string(rawResponse[18:(18 + ressponseLength)])
	} else {
		responseBody = ""
	}

	return responseBody
}
