package es.memdb.connection;

import java.io.IOException;
import java.util.function.Consumer;

public interface MemDbConnection extends AutoCloseable {
    void connect() throws IOException;

    byte[] read(int requestNumber);

    void write(byte[] value, int requestNumber);

    boolean isClosed();
}
