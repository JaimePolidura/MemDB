package es.memdb.connection;

import java.io.IOException;
import java.util.function.Consumer;

public interface MemDbConnection extends AutoCloseable{
    void connect() throws IOException;

    byte[] read(long requestNumber);

    void write(byte[] data);

    void write(byte[] data, Consumer<Byte[]> onResponseCallback);
}
