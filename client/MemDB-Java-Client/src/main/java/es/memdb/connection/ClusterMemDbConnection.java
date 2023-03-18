package es.memdb.connection;

import es.memdb.Utils;
import es.memdb.cluster.ClusterManager;
import es.memdb.cluster.Node;
import lombok.AllArgsConstructor;

import java.io.IOException;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.locks.Lock;
import java.util.function.Consumer;

@AllArgsConstructor
public class ClusterMemDbConnection implements MemDbConnection {
    private Map<Integer, Integer> onGoingClusterNodeIdByReqNumber = new ConcurrentHashMap<>();
    private Map<Integer, MemDbConnection> clusterConnectionsByNodeId = new ConcurrentHashMap<>();
    private Map<Integer, Lock> nodesToSendRequestLocksByNodeId = new ConcurrentHashMap<>();

    private final ClusterManager clusterManager;

    @Override
    public void write(byte[] requestBytes) {
        Map.Entry<Integer, MemDbConnection> pairToSendRequest = this.selectClusterNodeConnectionToSendRequest();
        MemDbConnection clusterNodeToSendRequest = pairToSendRequest.getValue();
        int nodeIdToSendRequest = pairToSendRequest.getKey();

        int requestNumber = Utils.toInt(requestBytes);

        clusterNodeToSendRequest.write(requestBytes);
        onGoingClusterNodeIdByReqNumber.put(requestNumber, nodeIdToSendRequest);
    }

    private Map.Entry<Integer, MemDbConnection> selectClusterNodeConnectionToSendRequest() {
        return null;
    }

    @Override
    public byte[] read(int requestNumber) {
        int nodeIdRequest = this.onGoingClusterNodeIdByReqNumber.get(requestNumber);
        MemDbConnection clusterNodeRequest = this.clusterConnectionsByNodeId.get(nodeIdRequest);

        byte[] response = clusterNodeRequest.read(requestNumber);

        this.nodesToSendRequestLocksByNodeId.get(nodeIdRequest).unlock();

        return response;
    }

    @Override
    public void write(byte[] value, Consumer<Byte[]> onResponseCallback) {
        throw new UnsupportedOperationException("xd");
    }

    @Override
    public boolean isClosed() {
        return this.clusterConnectionsByNodeId.values()
                .stream()
                .allMatch(MemDbConnection::isClosed);
    }

    @Override
    public void close() throws Exception {
        for (var clusterNodeConnection : this.clusterConnectionsByNodeId.entrySet()) {
            clusterNodeConnection.getValue().close();
        }
    }

    @Override
    public void connect() throws IOException {
        this.createNodeConnections();
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
}
