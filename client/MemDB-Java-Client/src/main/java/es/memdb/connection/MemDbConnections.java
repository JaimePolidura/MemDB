package es.memdb.connection;

import es.memdb.MemDb;
import es.memdb.cluster.ClusterManager;

import java.io.IOException;
import java.util.List;

public final class MemDbConnections {
    public static MemDbConnection cluster(String authApiKey, String... clusterManagerAddress) {
        return new ClusterMemDbSyncConnection(new ClusterManager(List.of(clusterManagerAddress), authApiKey));
    }

    public static MemDb sync(String host, int port, String authApiKey) throws IOException {
        return new MemDb(new SyncMemDbConnection(host, port), authApiKey, false);
    }
}
