package es.memdb.connection;

import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;

public final class ResponseReader {
    public byte[] read(InputStream input) {
        try{
            byte[] bufferLengthRespnose = new byte[4];
            input.read(bufferLengthRespnose);

            int responseLength = ByteBuffer.wrap(bufferLengthRespnose).getInt();

            byte[] bufferWithResponseBody = new byte[responseLength];

            input.read(bufferWithResponseBody);

            return bufferWithResponseBody;
        }catch (IOException e) {
            e.printStackTrace();
            throw new RuntimeException(e.getMessage());
        }
    }
}
