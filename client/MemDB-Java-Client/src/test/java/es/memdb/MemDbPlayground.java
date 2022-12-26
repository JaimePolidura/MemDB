package es.memdb;

import es.memdb.connection.MemDbConnections;
import lombok.SneakyThrows;

public final class MemDbPlayground {
    @SneakyThrows
    public static void main(String[] args) {
        MemDb memDb = new MemDb(MemDbConnections.async("127.0.0.1", 10000), "123");

        String response = memDb.set("nombre", "jaime");
        System.out.println("{Set nombre = jaime}");

        String value = memDb.get("nombre");
        System.out.println("{get nombre}: " + value);

        String result = memDb.get("xd");
        System.out.println("{get xd}: " + result);
    }
}
