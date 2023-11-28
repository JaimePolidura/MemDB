package es.memdb.connection;

import es.memdb.messages.request.Request;
import es.memdb.messages.request.RequestSerializer;
import es.memdb.messages.response.Response;
import es.memdb.messages.response.ResponseDeserializer;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public final class NoClusterMemDbConnection implements MemDbConnection {
    private final String host;
    private final int port;
    private Socket socket;
    private OutputStream output;
    private InputStream input;

    private final ResponseDeserializer responseDeserializer = new ResponseDeserializer();
    private final RequestSerializer requestSerializer = new RequestSerializer();
    private final ResponseReader responseReader = new ResponseReader();

    public NoClusterMemDbConnection(String host, int port) throws IOException {
        this.host = host;
        this.port = port;
        this.connect();
    }

    @Override
    public void connect() throws IOException {
        if(this.isClosed()){
            this.socket = new Socket(this.host, this.port);
            this.output = this.socket.getOutputStream();
            this.input = this.socket.getInputStream();
        }
    }

    @Override
    public Response send(Request request) {
        try {
            byte[] serialized = requestSerializer.serialize(request);
            output.write(serialized);

            byte[] bytesResponse = responseReader.read(input);
            return responseDeserializer.deserialize(bytesResponse);
        } catch (Exception e) {
            e.printStackTrace();
            throw new RuntimeException(e.getMessage());
        }
    }

    @Override
    public boolean isClosed() {
        return this.socket == null || this.socket.isClosed();
    }

    @Override
    public void close() throws IOException {
        this.socket.close();
    }
}
