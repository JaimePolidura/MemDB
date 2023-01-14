import es.memdb.MemDb;
import es.memdb.connection.MemDbConnections;
import lombok.SneakyThrows;

public final class Other {
    @SneakyThrows
    public static void main(String[] args) {
        MemDb memDb = new MemDb(MemDbConnections.sync("127.0.0.1", 10000), "123");
        memDb.delete("B");
    }
}
