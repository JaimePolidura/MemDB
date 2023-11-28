package es.memdb.connection;

import es.memdb.messages.request.Request;
import es.memdb.messages.response.Response;

import java.io.IOException;
import java.util.function.Consumer;

public interface MemDbConnection extends AutoCloseable {
    void connect() throws IOException;

    Response send(Request request);

    boolean isClosed();
}
