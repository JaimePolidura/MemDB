package es.memdb.connection;

import es.memdb.cluster.ClusterManager;

import java.io.IOException;
import java.util.List;

public final class MemDbConnections {
    public static MemDbConnection async(String host, int port) throws IOException {
        return new AsyncMemDbConnection(host, port);
    }

    public static MemDbConnection cluster(String authApiKey, String... address) throws IOException {
        return new ClusterMemDbSyncConnection(new ClusterManager(List.of(address), authApiKey));
    }

    public static MemDbConnection sync(String host, int port) throws IOException {
        return new SyncMemDbConnection(host, port);
    }
}
