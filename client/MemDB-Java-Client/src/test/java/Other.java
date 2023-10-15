import es.memdb.MemDb;
import es.memdb.connection.MemDbConnections;
import lombok.SneakyThrows;

public final class Other {
    @SneakyThrows
    public static void main(String[] args) {
        MemDb memDb = new MemDb(MemDbConnections.sync("192.168.1.159", 10000), "789");
        
//        memDb.set("dinero", "poco");
//        memDb.set("trabajo", "no tengo");
//        memDb.set("novia", "ninguna");
//        memDb.set("autistmo", "Pòsiblemente");

        System.out.println(memDb.get("dinero"));
    }
}
