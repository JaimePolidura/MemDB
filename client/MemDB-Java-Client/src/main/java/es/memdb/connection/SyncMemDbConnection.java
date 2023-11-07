package es.memdb.connection;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public final class SyncMemDbConnection implements MemDbConnection {
    private final String host;
    private final int port;
    private Socket socket;
    private OutputStream output;
    private InputStream input;

    private final ResponseReader responseReader = new ResponseReader();
    private final Lock operationLock = new ReentrantLock(true);

    public SyncMemDbConnection(String host, int port) throws IOException {
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
    public void write(byte[] value, int requestNumber) {
        try {
            this.operationLock.lock();

            this.output.write(value);
        } catch (IOException e) {
            this.operationLock.unlock();
            e.printStackTrace();
        }
    }

    @Override
    public boolean isClosed() {
        return this.socket == null || this.socket.isClosed();
    }

    @Override
    public byte[] read(int requestNumber) {
        try{
            return this.responseReader.read(this.input);
        } catch (Exception e) {
            e.printStackTrace();
            throw new RuntimeException(e.getMessage());
        }finally {
            this.operationLock.unlock();
        }
    }

    @Override
    public void close() throws IOException {
        this.socket.close();
    }
}
