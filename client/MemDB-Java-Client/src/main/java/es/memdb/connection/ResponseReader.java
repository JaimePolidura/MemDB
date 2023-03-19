package es.memdb.connection;

import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.util.Arrays;

public final class ResponseReader {
    public byte[] read(InputStream input) {
        try{
            byte[] bufferWithoutResponseBody = new byte[17];
            input.read(bufferWithoutResponseBody);
            int responseLength = ByteBuffer.wrap(bufferWithoutResponseBody, 13, 4).getInt();

            if(responseLength == 0)
                return bufferWithoutResponseBody;

            byte[] bufferWithResponseBody = Arrays.copyOf(bufferWithoutResponseBody, bufferWithoutResponseBody.length + responseLength);

            while (input.read(bufferWithResponseBody) != -1)
                return bufferWithResponseBody;

            return bufferWithResponseBody;
        }catch (IOException e) {
            e.printStackTrace();
            throw new RuntimeException(e.getMessage());
        }
    }
}
