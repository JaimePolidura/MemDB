package es.memdb.connection;

import es.memdb.cluster.ClusterManager;
import es.memdb.cluster.ClusterNodeConnection;
import es.memdb.cluster.Node;
import es.memdb.utils.CircularLockFreeMapIterator;
import io.vavr.control.Try;
import lombok.AllArgsConstructor;
import lombok.SneakyThrows;

import java.io.IOException;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.function.Consumer;

public class ClusterMemDbSyncConnection implements MemDbConnection {
    private Map<Integer, ClusterNodeConnection> clusterNodeConnections = new ConcurrentHashMap<>();
    private Map<Integer, Integer> onGoingClusterNodeIdByReqNumber = new ConcurrentHashMap<>();
    private Iterator<Map.Entry<Integer, ClusterNodeConnection>> nextNodeToSendRequestIterator;

    private final ClusterManager clusterManager;

    private final NodesChangeUpdaterTask nodesChangeUpdaterTask;

    public ClusterMemDbSyncConnection(ClusterManager clusterManager) {
        this.nodesChangeUpdaterTask = new NodesChangeUpdaterTask(clusterNodeConnections, clusterManager, this::onNodeUpdated,
                this::onNodeDeleted);
        this.clusterManager = clusterManager;

        this.setupOtherNodesList();

        this.nextNodeToSendRequestIterator = new CircularLockFreeMapIterator<>(this.clusterNodeConnections);
    }

    private void setupOtherNodesList() {
        this.clusterManager.getAllNodes().forEach(node -> Try.run(() -> {
            this.clusterNodeConnections.put(node.getNodeId(), ClusterNodeConnection.of(node));
        }));
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
    public void write(byte[] requestBytes, int requestNumber) {
        ClusterNodeConnection clusterNodeConnection = selectClusterNodeConnectionToSendRequest();
        boolean successWrite = writeToClusterNode(clusterNodeConnection, requestBytes, requestNumber);

        while (!successWrite) {
            removeNodeConnectionById(clusterNodeConnection);

            clusterNodeConnection = selectClusterNodeConnectionToSendRequest();
            successWrite = writeToClusterNode(clusterNodeConnection, requestBytes, requestNumber);
        }

        onGoingClusterNodeIdByReqNumber.put(requestNumber, clusterNodeConnection.getNode().getNodeId());
    }

    private boolean writeToClusterNode(ClusterNodeConnection clusterNodeConnection, byte[] bytes, int requestNumber) {
        return Try.run(() -> clusterNodeConnection.write(bytes, requestNumber)).isSuccess();
    }

    private ClusterNodeConnection selectClusterNodeConnectionToSendRequest() {
        if(this.clusterNodeConnections.isEmpty())
            throw new RuntimeException("No node available to send request");

        while (true) {
            //Iterator is lockfree, no race conditions
            Map.Entry<Integer, ClusterNodeConnection> nextNodeEntry = this.nextNodeToSendRequestIterator.next();

            return nextNodeEntry.getValue();
        }
    }

    @Override
    public byte[] read(int requestNumber) {
        int nodeIdRequest = this.onGoingClusterNodeIdByReqNumber.get(requestNumber);
        ClusterNodeConnection clusterNodeRequest = this.clusterNodeConnections.get(nodeIdRequest);

        return clusterNodeRequest.read(requestNumber);
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
