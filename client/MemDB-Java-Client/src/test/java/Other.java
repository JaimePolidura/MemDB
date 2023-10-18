import es.memdb.MemDb;
import es.memdb.connection.MemDbConnections;
import lombok.SneakyThrows;

public final class Other {
    @SneakyThrows
    public static void main(String[] args) {
        node1_write();
//        node4_read();
//        node1_read();
//        node5_read();
    }

    @SneakyThrows
    static void node1_write() {
        MemDb memDb = new MemDb(MemDbConnections.sync("192.168.1.159", 10000), "789");
        memDb.set("cascos", "avion");
    }

    @SneakyThrows
    static void node1_read() {
        MemDb memDb = new MemDb(MemDbConnections.sync("192.168.1.159", 10000), "789");
        System.out.println(memDb.get("cascos"));
    }

    @SneakyThrows
    static void node5_read() {
        MemDb memDb = new MemDb(MemDbConnections.sync("192.168.1.159", 10004), "789");
        System.out.println(memDb.get("cascos"));
    }

    @SneakyThrows
    static void node4_read() {
        MemDb memDb = new MemDb(MemDbConnections.sync("192.168.1.159", 10003), "789");
        System.out.println(memDb.get("cascos"));
    }

}


