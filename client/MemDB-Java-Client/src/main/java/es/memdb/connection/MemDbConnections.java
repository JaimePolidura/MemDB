package es.memdb.connection;

import java.io.IOException;

public final class MemDbConnections {
    public static MemDbConnection async(String host, int port) throws IOException {
        return new AsyncMemDbConnection(host, port);
    }
    
    public static MemDbConnection sync(String host, int port) throws IOException {
        return new SyncMemDbConnection(host, port);
    }
}
