import es.memdb.MemDb;
import es.memdb.connection.MemDbConnections;
import lombok.SneakyThrows;

public final class Other {
    @SneakyThrows
    public static void main(String[] args) {
        cast_test();
//        node1_write();
//        node3_write();
//        node5_write();
//        node1_read();
//        node4_read();
//        node5_read();
    }

    @SneakyThrows
    static void cast_test() {
        MemDb memDb = new MemDb(MemDbConnections.sync("192.168.1.159", 10001), "789");
        memDb.set("locked", "false");

        var success = memDb.cas("locked", "xd", "true");
        System.out.println(success);
    }

    @SneakyThrows
    static void node1_write() {
        MemDb memDb = new MemDb(MemDbConnections.sync("192.168.1.159", 10000), "789");
        memDb.set("cascos", "avion");
    }

    @SneakyThrows
    static void node5_write() {
        MemDb memDb = new MemDb(MemDbConnections.sync("192.168.1.159", 10004), "789");
        memDb.set("d", "xd");
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
    static void node3_write() {
        MemDb memDb = new MemDb(MemDbConnections.sync("192.168.1.159", 10002), "789");
        memDb.set("3", "Mi numero");
    }

    @SneakyThrows
    static void node4_read() {
        MemDb memDb = new MemDb(MemDbConnections.sync("192.168.1.159", 10003), "789");
        System.out.println(memDb.get("cascos"));
    }

}


