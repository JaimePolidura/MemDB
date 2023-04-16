package es.memdb.messages.response;

import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public final class ResponseDeserializer {
    private static final byte SUCCESSFUL_MASK = 0x01; //00000001

    //We assume that the raw comes without the "Total response length"
    public Response deserialize(byte[] raw) {
        int requestNumber = ByteBuffer.wrap(raw).getInt();
        long timestamp = ByteBuffer.wrap(raw, 4, 8).getLong();
        boolean isSuccessFul = (raw[4 + 8] & SUCCESSFUL_MASK) == 1;
        short errorCode = (short) (raw[4 + 8] >> 1);
        String response = (raw.length > (4 + 4 + 8 + 1) && raw[4 + 4 + 8 + 1 - 1] > 0) ?
                new String(this.split(raw, 4 + 4 + 8 + 1, raw.length), StandardCharsets.US_ASCII) :
                "";

        return Response.builder()
                .errorCode(errorCode)
                .requestNumber(requestNumber)
                .timestamp(timestamp)
                .isSuccessful(isSuccessFul)
                .response(formatResponse(response))
                .build();
    }

    private String formatResponse(String response) {
        return response.trim();
    }

    private byte[] split(byte[] source, int from, int to) {
        byte[] splitted = new byte[to - from];

        for(int i = from; i < to; i++){
            splitted[i - from] = source[i];
        }

        return splitted;
    }
}
