import es.memdb.MemDb;
import es.memdb.connection.MemDbConnections;
import lombok.SneakyThrows;

public final class Other {
    @SneakyThrows
    public static void main(String[] args) {
        MemDb memDb = new MemDb(MemDbConnections.sync("192.168.1.159", 10000), "789");

//        memDb.set("apellidos", "tu puta madre");
//        memDb.set("novia", "tu puta madre");
//        memDb.set("madre", "no tengo");
//        memDb.set("sexo", "helicoptero apache AH-64");

//        System.out.println(memDb.get("apellidos"));
    }
}
