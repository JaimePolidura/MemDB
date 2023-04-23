import es.memdb.MemDb;
import es.memdb.connection.MemDbConnections;
import lombok.SneakyThrows;

public final class Other {
    @SneakyThrows
    public static void main(String[] args) {
        MemDb memDb = new MemDb(MemDbConnections.cluster("789", "http://127.0.0.1:8080"), "789");

//        memDb.set("nombre", "jaime");
//        memDb.set("appelido", "polidura");
        System.out.println(memDb.get("appelido"));
        System.out.println(memDb.get("nombre"));
    }
}
