import es.memdb.MemDb;
import es.memdb.connection.MemDbConnections;
import lombok.SneakyThrows;

public final class Other {
    @SneakyThrows
    public static void main(String[] args) {
        MemDb memDb = new MemDb(MemDbConnections.sync("127.0.0.1", 10000), "123");

//        memDb.set("A", "1");
//        memDb.set("B", "1");
//        memDb.set("C", "1");
//
//        memDb.set("B", "2");
//
//        memDb.delete("J");
//        memDb.delete("A");

        /**
         * B -> 2
         * C -> 1
         */

        System.out.println(memDb.get("C"));
    }
}
