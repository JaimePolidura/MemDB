package es.memdb.connection;

import es.memdb.Utils;
import es.memdb.cluster.ClusterManager;
import es.memdb.cluster.ClusterNodeConnection;
import es.memdb.cluster.Node;
import lombok.AllArgsConstructor;
import lombok.SneakyThrows;

import java.io.IOException;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.function.Consumer;

public class ClusterMemDbSyncConnection implements MemDbConnection {
    private Map<Integer, AtomicBoolean> nodesToSendRequestLocksByNodeId = new ConcurrentHashMap<>();
    private Map<Integer, ClusterNodeConnection> clusterNodeConnections = new ConcurrentHashMap<>();
    private Map<Integer, Integer> onGoingClusterNodeIdByReqNumber = new ConcurrentHashMap<>();

    private final ClusterManager clusterManager;

    private final NodesChangeUpdaterTask nodesChangeUpdaterTask;

    public ClusterMemDbSyncConnection(ClusterManager clusterManager) {
        this.clusterManager = clusterManager;
        this.nodesChangeUpdaterTask = new NodesChangeUpdaterTask(clusterNodeConnections, clusterManager,
                this::onNodeUpdated, this::onNodeDeleted);
    }

    @SneakyThrows
    private void onNodeUpdated(Node node) {
        if(this.clusterNodeConnections.containsKey(node.getNodeId())){
            this.clusterNodeConnections.get(node.getNodeId()).connect();
        }else{
            MemDbConnection nodeConnection = createConnection(node);
            this.clusterNodeConnections.put(node.getNodeId(), new ClusterNodeConnection(nodeConnection, node));
        }
    }

    private void onNodeDeleted(Node node) {
        var removedNode = this.clusterNodeConnections.remove(node.getNodeId());
        if(removedNode != null)
            removedNode.close();
    }

    @Override
    public void write(byte[] requestBytes) {
        ClusterNodeConnection clusterNodeConnection = selectClusterNodeConnectionToSendRequest();
        boolean successWrite = writeToClusterNode(clusterNodeConnection, requestBytes);

        while (!successWrite) {
            removeNodeConnectionById(clusterNodeConnection);

            clusterNodeConnection = selectClusterNodeConnectionToSendRequest();
            successWrite = writeToClusterNode(clusterNodeConnection, requestBytes);
        }

        int requestNumber = Utils.toInt(requestBytes);
        onGoingClusterNodeIdByReqNumber.put(requestNumber, clusterNodeConnection.getNode().getNodeId());
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
            int nodeId = clusterNodeConnection.getNode().getNodeId();

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
        throw new UnsupportedOperationException();
    }

    @Override
    public boolean isClosed() {
        return this.clusterNodeConnections.values().stream()
                .allMatch(ClusterNodeConnection::isClosed);
    }

    @Override
    public void close() {
        for (var clusterNodeConnection : this.clusterNodeConnections.values()) {
            clusterNodeConnection.close();
        }
    }

    @Override
    public void connect() throws IOException {
        this.createNodeConnections();
        Executors.newScheduledThreadPool(1).schedule(this.nodesChangeUpdaterTask, 1, TimeUnit.MINUTES);
    }

    private void createNodeConnections() throws IOException {
        List<Node> nodes = this.clusterManager.getAllNodes();

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
        this.clusterNodeConnections.remove(clusterNodeConnectionToRemove.getNode().getNodeId());
        clusterNodeConnectionToRemove.close();
    }

    @AllArgsConstructor
    private static class NodesChangeUpdaterTask implements Runnable {
        private Map<Integer, ClusterNodeConnection> clusterNodeConnections;
        private ClusterManager clusterManager;
        private Consumer<Node> onNodeChanged;
        private Consumer<Node> onNodeDeleted;

        @Override
        public void run() {
            for (Node nodeFromClusterDb : this.clusterManager.getAllNodes()) {
                if (nodeFromClusterDb.getState().canSendRequest && nodeChangedNotPresent(nodeFromClusterDb))
                    this.onNodeChanged.accept(nodeFromClusterDb);
                else if (!nodeFromClusterDb.getState().canSendRequest)
                    this.onNodeDeleted.accept(nodeFromClusterDb);
            }
        }

        private boolean nodeChangedNotPresent(Node nodeFromClusterDb) {
            var alreadyCreatedClusterNodeConnection = this.clusterNodeConnections.get(nodeFromClusterDb.getNodeId())
                    .getNode();

            return alreadyCreatedClusterNodeConnection == null ||
                    !alreadyCreatedClusterNodeConnection.equals(nodeFromClusterDb);
        }
    }
}
