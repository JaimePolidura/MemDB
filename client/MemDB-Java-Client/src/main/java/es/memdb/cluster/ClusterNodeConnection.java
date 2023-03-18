package es.memdb.cluster;

import es.memdb.connection.MemDbConnection;
import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.SneakyThrows;

@AllArgsConstructor
public final class ClusterNodeConnection {
    @Getter private final MemDbConnection memDbConnection;
    @Getter private final int nodeId;

    public void write(byte[] bytes) {
        this.memDbConnection.write(bytes);
    }

    public byte[] read(int requestNumber) {
        return this.memDbConnection.read(requestNumber);
    }

    @SneakyThrows
    public void close() {
        this.memDbConnection.close();
    }

    public boolean isClosed(){
        return this.memDbConnection.isClosed();
    }
}
