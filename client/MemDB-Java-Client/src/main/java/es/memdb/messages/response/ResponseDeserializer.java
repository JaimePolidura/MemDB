package es.memdb.messages.response;

import lombok.SneakyThrows;

import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;

public final class ResponseDeserializer {
    private static final byte SUCCESSFUL_MASK = 0x01; //00000001

    @SneakyThrows
    public Response deserialize(byte[] raw) {
        boolean isSuccessFul = (raw[8 + 8] & SUCCESSFUL_MASK) == 1;
        short errorCode = (short) (raw[8 + 8] >> 1);
        long timestamp = ByteBuffer.wrap(raw, 8, 8 + 8).getLong();
        String response = (raw.length > (1 + 8 + 8) &&  raw[8 + 8] > 0) ?
                new String(this.split(raw, 8 + 8, raw.length), StandardCharsets.US_ASCII) :
                "";

        return Response.builder()
                .errorCode(errorCode)
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
