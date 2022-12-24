package es.memdb;

import lombok.RequiredArgsConstructor;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;

@RequiredArgsConstructor
public final class MemDbConnection {
    private final String host;
    private final int port;
    private Socket socket;
    private OutputStream output;
    private InputStream input;

    private final byte[] buffer = new byte[257];

    public void connect() throws IOException {
        this.socket = new Socket(this.host, this.port);
        this.output = this.socket.getOutputStream();
        this.input = this.socket.getInputStream();
    }

    public void write(byte[] data) {
        try {
            this.output.write(data);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public byte[] read() {
        try{
            while (this.input.read(buffer) != -1)
                return this.buffer;

            return this.buffer;
        }catch (IOException e) {
            e.printStackTrace();
            throw new RuntimeException(e.getMessage());
        }
    }

    public boolean isOpen() {
        return this.socket != null && !this.socket.isClosed();
    }

    public void close() throws IOException {
        this.socket.close();
    }
}
