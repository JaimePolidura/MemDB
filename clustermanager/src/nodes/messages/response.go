package messages

type Response struct {
	ErrorCode    uint8
	Success      bool
	ResponseBody string
}
