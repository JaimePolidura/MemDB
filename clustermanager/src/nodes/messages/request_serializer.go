package messages

import (
	"bytes"
	"encoding/binary"
)

func SerializeRequest(request Request) []byte {
	serialized := make([]byte, 0)

	var buf bytes.Buffer
	binary.Write(&buf, binary.BigEndian, request.GetTotalLength())

	serialized = append(serialized, 0x00)                                           //Fragmentation header
	serialized = append(serialized, buf.Bytes()...)                                 //Request length
	serialized = append(serialized, 0x00, 0x00, 0x00, 0x00)                         //Req number
	serialized = append(serialized, byte(len(request.authKey)<<2))                  //Auth size
	serialized = append(serialized, []byte(request.authKey)...)                     //Auth key
	serialized = append(serialized, request.operatorNumber<<2)                      //Op number
	serialized = append(serialized, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00) //Timestamp
	serialized = append(serialized, request.args...)

	return serialized
}
