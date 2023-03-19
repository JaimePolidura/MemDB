package es.memdb.connection;

import es.memdb.cluster.ClusterManager;

import java.io.IOException;
import java.util.List;

public final class MemDbConnections {
    public static MemDbConnection async(String host, int port) throws IOException {
        return new AsyncMemDbConnection(host, port);
    }

    public static MemDbConnection cluster(String authClusterKey, String clusterManagerAddress) throws IOException {
        return new ClusterMemDbSyncConnection(new ClusterManager(List.of(clusterManagerAddress), authClusterKey));
    }

    public static MemDbConnection sync(String host, int port) throws IOException {
        return new SyncMemDbConnection(host, port);
    }
}
