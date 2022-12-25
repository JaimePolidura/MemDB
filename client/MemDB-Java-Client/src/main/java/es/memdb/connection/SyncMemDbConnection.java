package es.memdb.connection;

import es.memdb.Utils;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.function.Consumer;

public final class SyncMemDbConnection implements MemDbConnection {
    private final String host;
    private final int port;
    private Socket socket;
    private OutputStream output;
    private InputStream input;

    private final byte[] buffer = new byte[257];
    private final Lock operationLock = new ReentrantLock(true);

    public SyncMemDbConnection(String host, int port) throws IOException {
        this.host = host;
        this.port = port;
        this.connect();
    }

    @Override
    public void connect() throws IOException {
        this.socket = new Socket(this.host, this.port);
        this.output = this.socket.getOutputStream();
        this.input = this.socket.getInputStream();
    }

    @Override
    public void write(byte[] data) {
        try {
            this.operationLock.lock();

            this.output.write(data);
        } catch (IOException e) {
            e.printStackTrace();
        }finally {
            this.operationLock.unlock();
        }
    }

    @Override
    public void write(byte[] data, Consumer<Byte[]> onResponseCallback) {
        try {
            this.operationLock.lock();

            long requestNumber = Utils.toLong(data);

            this.output.write(data);

            onResponseCallback.accept(Utils.primitiveToWrapper(this.read(requestNumber)));
        } catch (IOException e) {
            e.printStackTrace();
        }finally {
            this.operationLock.unlock();
        }
    }

    @Override
    public byte[] read(long requestNumber) {
        try{
            while (this.input.read(buffer) != -1)
                return this.buffer;

            return this.buffer;
        }catch (IOException e) {
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
