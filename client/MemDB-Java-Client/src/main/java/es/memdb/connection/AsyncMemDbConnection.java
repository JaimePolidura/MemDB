package es.memdb.connection;

import es.memdb.Utils;
import lombok.SneakyThrows;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.util.Map;
import java.util.concurrent.*;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.function.Consumer;

public final class AsyncMemDbConnection implements MemDbConnection {
    private final String host;
    private final int port;
    private Socket socket;
    private OutputStream output;
    private InputStream input;

    private final Executor callbackExecutorThreadPool = Executors.newFixedThreadPool(Runtime.getRuntime().availableProcessors());
    private final Map<Integer, WaitReadResponseCondition> readMutex = new ConcurrentHashMap<>();
    private final Map<Integer, Byte[]> requestWithoutCallbacks = new ConcurrentHashMap<>();
    private final Map<Integer, Consumer<Byte[]>> requestWithCallbacks = new ConcurrentHashMap<>();
    private Lock writeSocketLock = new ReentrantLock(true);

    private final ServerAsyncReader serverAsyncReader = new ServerAsyncReader();
    private final ServerAsyncWriter serverAsyncWriter = new ServerAsyncWriter();

    public AsyncMemDbConnection(String host, int port) throws IOException {
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

            this.serverAsyncReader.start();
            this.serverAsyncWriter.start();
        }
    }

    @Override
    public void close() throws IOException {
        this.socket.close();
    }

    @Override
    @SneakyThrows
    public byte[] read(int requestNumber) {
        Byte[] value = this.requestWithoutCallbacks.get(requestNumber);

        if(value == null){
            WaitReadResponseCondition waitRead = this.readMutex.get(requestNumber);

            waitRead.lock();
            while((value = this.requestWithoutCallbacks.get(requestNumber)) == null) {
                waitRead.await();
            }
            waitRead.unlock();

            this.readMutex.remove(requestNumber);
        }

        return Utils.wrapperToPrimitive(value);
    }

    @Override
    public void write(byte[] value, int requestNumber) {
        try {
            Lock lock = new ReentrantLock();
            Condition condition = lock.newCondition();
            this.readMutex.put(requestNumber, new WaitReadResponseCondition(lock, condition));

            this.writeToStream(value);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void write(byte[] value, int requestNumber, Consumer<Byte[]> onResponseCallback) {
        try {
            this.writeToStream(value);

            this.requestWithCallbacks.put(requestNumber, onResponseCallback);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @Override
    public boolean isClosed() {
        return this.socket == null || this.socket.isClosed();
    }

    private void writeToStream(byte[] toWrite) throws IOException {
        try {
            this.writeSocketLock.lock();
            this.serverAsyncWriter.enqueue(toWrite);
        } finally {
            this.writeSocketLock.unlock();
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

    private class ServerAsyncReader extends Thread {
        private final ResponseReader responseReader = new ResponseReader();

        @Override
        public void run() {
            while (!socket.isClosed()) {
                byte[] fromBufferRaw = this.responseReader.read(input);
                Byte[] fromBufferNotRaw = Utils.primitiveToWrapper(fromBufferRaw);

                int requestNumber = Utils.toInt(fromBufferRaw);

                Consumer<Byte[]> callback = requestWithCallbacks.get(requestNumber);

                if(callback != null) {
                    callbackExecutorThreadPool.execute(() -> callback.accept(fromBufferNotRaw));
                }else{
                    requestWithoutCallbacks.put(requestNumber, fromBufferNotRaw);

                    WaitReadResponseCondition readResponseCondition = readMutex.get(requestNumber);

                    readResponseCondition.lock();
                    readResponseCondition.signalAll();
                    readResponseCondition.unlock();
                }
            }
        }
    }

    private class ServerAsyncWriter extends Thread {
        private final BlockingQueue<Byte[]> requestToWrite;

        public ServerAsyncWriter() {
            this.requestToWrite = new LinkedBlockingQueue<>();
        }

        public void enqueue(byte[] data) {
            this.requestToWrite.add(Utils.primitiveToWrapper(data));
        }

        @Override
        @SneakyThrows
        public void run() {
            while (!socket.isClosed()) {
                Byte[] requestToWrite = this.requestToWrite.take();

                output.write(Utils.wrapperToPrimitive(requestToWrite));
                output.flush();

                Thread.yield();
            }
        }
    }
}
