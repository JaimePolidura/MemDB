package es.memdb.cluster;

import es.memdb.connection.MemDbConnection;
import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.SneakyThrows;

import java.util.Objects;

@AllArgsConstructor
public final class ClusterNodeConnection {
    @Getter private final MemDbConnection memDbConnection;
    @Getter private final Node node;

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        ClusterNodeConnection that = (ClusterNodeConnection) o;
        return Objects.equals(node, that.node);
    }

    @Override
    public int hashCode() {
        return Objects.hash(node);
    }

    public void write(byte[] bytes) {
        this.memDbConnection.write(bytes);
    }

    public byte[] read(int requestNumber) {
        return this.memDbConnection.read(requestNumber);
    }

    @SneakyThrows
    public void connect() {
        this.memDbConnection.connect();
    }

    @SneakyThrows
    public void close() {
        this.memDbConnection.close();
    }

    public boolean isClosed(){
        return this.memDbConnection.isClosed();
    }
}
