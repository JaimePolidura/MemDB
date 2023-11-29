package es.memdb.connection;

import es.memdb.MemDb;
import es.memdb.cluster.ClusterMemDbConnection;

import java.io.IOException;
import java.util.List;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;

public final class MemDbConnections {
    public static MemDb cluster(String authUserKey, List<String> seedNodesAddress, Executor requestExecutor) {
        return new MemDb(new ClusterMemDbConnection(seedNodesAddress, authUserKey, requestExecutor), authUserKey, true);
    }

    public static MemDb cluster(String authUserKey, List<String> seedNodesAddress) {
        return new MemDb(new ClusterMemDbConnection(seedNodesAddress, authUserKey, Executors.newFixedThreadPool(20)), authUserKey, true);
    }

    public static MemDb sync(String host, int port, String authUserKey) throws IOException {
        return new MemDb(new NoClusterMemDbConnection(host, port), authUserKey, false);
    }
}
