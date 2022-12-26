package es.memdb.connection;

import es.memdb.Utils;
import lombok.SneakyThrows;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.function.Consumer;

public final class AsyncMemDbConnection implements MemDbConnection, Runnable {
    private final String host;
    private final int port;
    private Socket socket;
    private OutputStream output;
    private InputStream input;

    private final byte[] buffer = new byte[257];

    private final Map<Long, Byte[]> requestWithoutCallbacks;
    private final Map<Long, Consumer<Byte[]>> callbacks;
    private final Executor callbackExecutorThreadPool;
    private final Map<Long, WaitReadResponseCondition> readMutex;
    private final Thread bufferReaderThread;

    public AsyncMemDbConnection(String host, int port) throws IOException {
        this.callbackExecutorThreadPool = Executors.newFixedThreadPool(Runtime.getRuntime().availableProcessors());
        this.requestWithoutCallbacks = new ConcurrentHashMap<>();
        this.callbacks = new ConcurrentHashMap<>();
        this.readMutex = new ConcurrentHashMap<>();
        this.bufferReaderThread = new Thread(this);
        this.host = host;
        this.port = port;
        this.connect();
    }

    @Override
    public void connect() throws IOException {
        this.socket = new Socket(this.host, this.port);
        this.output = this.socket.getOutputStream();
        this.input = this.socket.getInputStream();
        this.bufferReaderThread.start();
    }

    @Override
    public void close() throws IOException {
        this.socket.close();
    }

    @Override
    @SneakyThrows
    public byte[] read(long requestNumber) {
        Byte[] data = this.requestWithoutCallbacks.get(requestNumber);

        if(data == null){
            WaitReadResponseCondition waitRead = this.readMutex.get(requestNumber);

            waitRead.lock();
            while((data = this.requestWithoutCallbacks.get(requestNumber)) == null)
                waitRead.await();
            waitRead.unlock();

            this.readMutex.remove(requestNumber);
        }

        return Utils.wrapperToPrimitive(data);
    }

    @Override
    public void write(byte[] data) {
        try {
            long requestNumber = Utils.toLong(data);

            Lock lock = new ReentrantLock();
            Condition condition = lock.newCondition();
            this.readMutex.put(requestNumber, new WaitReadResponseCondition(lock, condition));

            this.output.write(data);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void write(byte[] data, Consumer<Byte[]> onResponseCallback) {
        try {
            this.output.write(data);

            long requestNumber = Utils.toLong(data);
            this.callbacks.put(requestNumber, onResponseCallback);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @Override
    @SneakyThrows
    public void run() {
        while (!this.socket.isClosed()) {
            byte[] fromBufferRaw = this.read();
            Byte[] fromBufferNotRaw = Utils.primitiveToWrapper(fromBufferRaw);

            long requestNumber = Utils.toLong(fromBufferRaw);

            Consumer<Byte[]> callback = this.callbacks.get(requestNumber);

            if(callback != null) {
                this.callbackExecutorThreadPool.execute(() -> callback.accept(fromBufferNotRaw));
            }else{
                this.requestWithoutCallbacks.put(requestNumber, fromBufferNotRaw);

                WaitReadResponseCondition readResponseCondition = this.readMutex.get(requestNumber);

                readResponseCondition.lock();
                readResponseCondition.signalAll();
                readResponseCondition.unlock();
            }
        }
    }

    private byte[] read() {
        try{
            while (this.input.read(buffer) != -1)
                return this.buffer;

            return this.buffer;
        }catch (IOException e) {
            e.printStackTrace();
            throw new RuntimeException(e.getMessage());
        }
    }

    private record WaitReadResponseCondition(Lock lockOfCondition, Condition condition) {
        public void lock() {
            this.lockOfCondition.lock();
        }

        public void unlock() {
            this.lockOfCondition.unlock();
        }

        public void signalAll() {
            this.condition.signalAll();
        }

        @SneakyThrows
        public void await() {
            this.condition.await();
        }
    }

}
