package es.memdb.connection;

import es.memdb.Utils;
import es.memdb.cluster.ClusterManager;
import es.memdb.cluster.ClusterNodeConnection;
import es.memdb.cluster.Node;
import lombok.AllArgsConstructor;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.function.Consumer;

@AllArgsConstructor
public class ClusterMemDbSyncConnection implements MemDbConnection {
    private Map<Integer, AtomicBoolean> nodesToSendRequestLocksByNodeId = new ConcurrentHashMap<>();
    private Map<Integer, Integer> onGoingClusterNodeIdByReqNumber = new ConcurrentHashMap<>();
    private List<ClusterNodeConnection> clusterNodeConnections = new ArrayList<>();

    private final ClusterManager clusterManager;

    private NodesChangeUpdaterTask nodesChangeUpdaterTask = new NodesChangeUpdaterTask(clusterNodeConnections, clusterManager);

    @Override
    public void write(byte[] requestBytes) {
        ClusterNodeConnection clusterNodeConnection = this.selectClusterNodeConnectionToSendRequest();
        boolean successWrite = this.writeToClusterNode(clusterNodeConnection, requestBytes);

        while (!successWrite) {
            removeNodeConnectionById(clusterNodeConnection);

            clusterNodeConnection = selectClusterNodeConnectionToSendRequest();
            successWrite = writeToClusterNode(clusterNodeConnection, requestBytes);
        }

        int requestNumber = Utils.toInt(requestBytes);
        onGoingClusterNodeIdByReqNumber.put(requestNumber, clusterNodeConnection.getNodeId());
    }

    private boolean writeToClusterNode(ClusterNodeConnection clusterNodeConnection, byte[] bytes) {
        try {
            clusterNodeConnection.write(bytes);
            return true;
        }catch (Exception e) {
            return false;
        }
    }

    private ClusterNodeConnection selectClusterNodeConnectionToSendRequest() {
        int positionToTakeNode = (int) (Math.random() * this.clusterNodeConnections.size());

        while (true) {
            ClusterNodeConnection clusterNodeConnection = this.clusterNodeConnections.get(positionToTakeNode);
            int nodeId = clusterNodeConnection.getNodeId();

            if(this.nodesToSendRequestLocksByNodeId.get(nodeId).compareAndExchange(false, true))
                return clusterNodeConnection;

            positionToTakeNode = positionToTakeNode + 1 != this.clusterNodeConnections.size() ? positionToTakeNode + 1 :  0;
        }
    }

    @Override
    public byte[] read(int requestNumber) {
        int nodeIdRequest = this.onGoingClusterNodeIdByReqNumber.get(requestNumber);
        ClusterNodeConnection clusterNodeRequest = this.clusterNodeConnections.get(nodeIdRequest);

        byte[] response = clusterNodeRequest.read(requestNumber);

        this.nodesToSendRequestLocksByNodeId.get(nodeIdRequest).set(false);

        return response;
    }

    @Override
    public void write(byte[] value, Consumer<Byte[]> onResponseCallback) {
        throw new UnsupportedOperationException("xd");
    }

    @Override
    public boolean isClosed() {
        return this.clusterNodeConnections.stream()
                .allMatch(ClusterNodeConnection::isClosed);
    }

    @Override
    public void close() throws Exception {
        for (var clusterNodeConnection : this.clusterNodeConnections) {
            clusterNodeConnection.close();
        }
    }

    @Override
    public void connect() throws IOException {
        this.createNodeConnections();
        Executors.newScheduledThreadPool(1).schedule(this.nodesChangeUpdaterTask, 1, TimeUnit.MINUTES);
    }

    private void createNodeConnections() throws IOException {
        List<Node> nodes = this.clusterManager.getAllNodes().getNodes();

        for (Node node : nodes) {
            this.createConnection(node);
        }
    }

    private MemDbConnection createConnection(Node node) throws IOException {
        String[] addressSplitted = node.getAddress().split(":");
        String address = addressSplitted[0];
        String port = addressSplitted[1];

        SyncMemDbConnection nodeConnection = new SyncMemDbConnection(address, Integer.parseInt(port));
        nodeConnection.connect();

        return nodeConnection;
    }

    private void removeNodeConnectionById(ClusterNodeConnection clusterNodeConnectionToRemove) {
        clusterNodeConnectionToRemove.close();
        this.clusterNodeConnections.removeIf(clusterNodeConnection -> clusterNodeConnection.getNodeId() == clusterNodeConnectionToRemove.getNodeId());
    }

    @AllArgsConstructor
    private static class NodesChangeUpdaterTask implements Runnable {
        private List<ClusterNodeConnection> clusterNodeConnections;
        private ClusterManager clusterManager;

        @Override
        public void run() {
        }
    }
}
