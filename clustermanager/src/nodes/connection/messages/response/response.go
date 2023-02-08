package response

type Response struct {
	errorCode    uint8
	success      bool
	responseBody string
}
