package es.memdb.connection;

import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;

public final class ResponseReader {
    public byte[] read(InputStream input) {
        try{
            byte[] fragmentationHeader = new byte[1];
            input.read(fragmentationHeader);
            if(fragmentationHeader[0] != 0x00){
                throw new UnsupportedOperationException("Client library doest support fragmented responses");
            }

            byte[] bufferLengthResponse = new byte[4];
            input.read(bufferLengthResponse);

            int responseLength = ByteBuffer.wrap(bufferLengthResponse).getInt();

            byte[] bufferWithResponseBody = new byte[responseLength];

            input.read(bufferWithResponseBody);

            return bufferWithResponseBody;
        }catch (IOException e) {
            e.printStackTrace();
            throw new RuntimeException(e.getMessage());
        }
    }
}
