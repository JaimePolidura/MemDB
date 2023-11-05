import es.memdb.MemDb;
import es.memdb.connection.MemDbConnections;
import lombok.SneakyThrows;

public final class Other {
    @SneakyThrows
    public static void main(String[] args) {
//        concurrent_cas();
//        simple_cas();
        linearized_cas();
//        node1_write();
//        node3_write();
//        node5_write();
//        node1_read();
//        node4_read();
//        node5_read();
    }

    @SneakyThrows
    static void linearized_cas() {
        MemDb memDb = new MemDb(MemDbConnections.sync("192.168.1.159", 10000), "789");
        memDb.set("n", "0");

        for(int i = 0; i < 100; i++){
            while (!memDb.cas("n", String.valueOf(i), String.valueOf(i + 1))) {
                System.out.println("Sleeping");
                Thread.sleep(1000L * 60);
            }
        }

        System.out.println("FINAL!");

    }

    @SneakyThrows
    static void concurrent_cas() {
        MemDb globalMemDb = new MemDb(MemDbConnections.sync("192.168.1.159", 10001), "789");
        globalMemDb.set("lock", "false");

        Thread t1 = new Thread(() -> {
            try {
                MemDb local = new MemDb(MemDbConnections.sync("192.168.1.159", 10000), "789");
                for(int i = 0; i < 100; i++){
                    while(!local.cas("lock", "false", "true")) {
                        Thread.sleep(100);
                    }

                    String contadorString = local.get("contador");
                    int contadorInt = contadorString != null ? Integer.parseInt(contadorString) : 0;

                    local.set("contador", String.valueOf(contadorInt + 1));

                    local.set("lock", "false");
                }

                System.out.println(local.cas("locked", "false", "true"));
            } catch (Exception e) {
                throw new RuntimeException(e);
            }
        });

        Thread t2 = new Thread(() -> {
            try {
                MemDb local = new MemDb(MemDbConnections.sync("192.168.1.159", 10001), "789");
                for(int i = 0; i < 100; i++){
                    while(!local.cas("lock", "false", "true")) {
                        Thread.sleep(100);
                    }

                    String contadorString = local.get("contador");
                    int contadorInt = contadorString != null ? Integer.parseInt(contadorString) : 0;

                    local.set("contador", String.valueOf(contadorInt + 1));

                    local.set("lock", "false");
                }

                System.out.println(local.cas("locked", "false", "true"));
            } catch (Exception e) {
                throw new RuntimeException(e);
            }
        });

        t1.start();
        t2.start();

        t1.join();
        t1.join();

        System.out.println(globalMemDb.get("contador"));
    }

    @SneakyThrows
    static void simple_cas() {
        MemDb memDb = new MemDb(MemDbConnections.sync("192.168.1.159", 10000), "789");
        memDb.set("locked", "false");

        for(int i = 0; i < 100; i++){
            while (!memDb.cas("locked", "false", "true")) {
                Thread.sleep(1000L * 60);
            }

            memDb.set("locked", "false");
        }

        System.out.println("FINAL!");
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


