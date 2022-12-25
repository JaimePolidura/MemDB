package es.memdb.messages.response;

import lombok.SneakyThrows;

import java.nio.charset.StandardCharsets;

public final class ResponseDeserializer {
    private static final byte SUCCESSFUL_MASK = 0x01; //00000001

    @SneakyThrows
    public Response deserialize(byte[] raw) {
        boolean isSuccessFul = (raw[0] & SUCCESSFUL_MASK) == 1;
        short errorCode = (short) (raw[0] >> 1);
        String response = (raw.length > 1 &&  raw[1] > 0) ?
                new String(this.split(raw, 1, raw.length), StandardCharsets.US_ASCII) :
                "";

        return Response.builder()
                .errorCode(errorCode)
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
