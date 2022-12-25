package es.memdb;

import es.memdb.connection.SyncMemDbConnection;
import lombok.SneakyThrows;

public final class MemDbPlayground {
    @SneakyThrows
    public static void main(String[] args) {
        MemDb memDb = new MemDb(new SyncMemDbConnection("127.0.0.1", 10000), "123");

        String response = memDb.set("nombre", "jaime");
        System.out.println("Set");

        String value = memDb.get("nombre");
        System.out.println(value);

        String result = memDb.get("xd");
        System.out.println(result);
    }
}
