package es.memdb;

import es.memdb.connection.MemDbConnections;
import lombok.SneakyThrows;

public final class MemDbPlayground {
    @SneakyThrows
    public static void main(String[] args) {
        MemDb memDb = MemDbConnections.sync("127.0.0.1", 10000, "123");

        memDb.set("nombre", "jaime");
        System.out.println("{Set nombre = jaime}");

        System.out.println("{get nombre}: " + memDb.get("nombre"));

        String resultGetxd = memDb.get("xd");
        System.out.println("{get xd}: " + resultGetxd);

        memDb.delete("xd");
        System.out.println("{delete xd}");

        memDb.delete("nombre");
        System.out.println("{delete nombre}");

        String resultGetNombre = memDb.get("nombre");
        System.out.println("{get nombre}: " + resultGetNombre);
    }
}
