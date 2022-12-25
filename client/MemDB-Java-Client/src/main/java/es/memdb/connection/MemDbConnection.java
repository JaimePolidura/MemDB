package es.memdb.connection;

import java.io.IOException;
import java.util.function.Consumer;

public interface MemDbConnection {
    void connect() throws IOException;

    void close() throws IOException;

    byte[] read(long requestNumber);

    void write(byte[] data);

    void write(byte[] data, Consumer<Byte[]> onResponseCallback);
}
