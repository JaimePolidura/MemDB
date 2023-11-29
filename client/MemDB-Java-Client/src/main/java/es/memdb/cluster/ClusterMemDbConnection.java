package es.memdb.cluster;

import es.memdb.Operator;
import es.memdb.Utils;
import es.memdb.connection.MemDbConnection;
import es.memdb.messages.request.AuthenticationRequest;
import es.memdb.messages.request.OperationRequest;
import es.memdb.messages.request.Request;
import es.memdb.messages.response.MultiResponses;
import es.memdb.messages.response.Response;
import io.vavr.control.Try;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.StandardCharsets;
import java.util.*;
import java.util.concurrent.Executor;
import java.util.function.Function;
import java.util.stream.Collectors;

public final class ClusterMemDbConnection implements MemDbConnection {
    private Map<Integer, Node> nodesById;
    private List<Node> seedNodes;
    private List<Node> allNodes;

    private Executor requestThreadPool;

    private String authUserKey;

    private ClusterConfig clusterConfig;
    private RingEntries ringEntries;

    private boolean isClosed;

    public ClusterMemDbConnection(List<String> seedNodesAddress, String authUserKey, Executor requestThreadPool) {
        List<Node> seedNodes = seedNodesAddress.stream()
                .map(Node::new)
                .toList();
        this.requestThreadPool = requestThreadPool;
        this.nodesById = seedNodes.stream().collect(Collectors.toMap(Node::getNodeId, Function.identity()));
        this.allNodes = new ArrayList<>(seedNodes);
        this.authUserKey = authUserKey;
        this.seedNodes = seedNodes;
        this.isClosed = false;
    }

    public MultiResponses sendMultipleRequest(Request request) {
        boolean usingPartition = request.hasOneArg() && clusterConfig.usingPartitions();
        List<Node> nodesToSendMulti = usingPartition ? getNodesInPartitionByKey(request.getFirstArg()) : allNodes;
        int nNodesToSendMulti = usingPartition ? clusterConfig.nodesPerPartition() : allNodes.size();

        MultiResponses multiResponses = MultiResponses.fromCluster(nNodesToSendMulti);
        MultiResponses.MultipleResponseNotifier notifier = new MultiResponses.MultipleResponseNotifier(multiResponses);
        List<Node> nodesToSendRequest = Utils.getRandomUnique(nodesToSendMulti, nNodesToSendMulti);

        for (int i = 0; i < nNodesToSendMulti; i++) {
            Node node = nodesToSendRequest.get(i);

            requestThreadPool.execute(() -> {
                notifier.addResponse(node.sendRequest(request));
            });
        }

        return multiResponses;
    }

    @Override
    public void connect() throws IOException {
        if(clusterConfig != null){
            loadClusterConfig();
        }
    }

    @Override
    public Response send(Request request) {
        Utils.rethrowNoChecked(this::connect);

        boolean hasKey = !request.getOperationRequest().getArgs().isEmpty();

        if(hasKey && clusterConfig.usingPartitions()) {
            return sendRequestToPartition(request);
        } else {
            return sendRequestToRandomNode(allNodes, request);
        }
    }

    @Override
    public boolean isClosed() {
        return isClosed;
    }

    @Override
    public void close() throws Exception {
        this.allNodes.forEach(Node::connect);
    }

    private Response sendRequestToPartition(Request request) {
        while(true){
            String key = request.getOperationRequest().getArgs().get(0);
            List<Node> nodesPartition = getNodesInPartitionByKey(key);

            Response response = sendRequestToRandomNode(nodesPartition, request);
            if (response.hasError(6)) { //Invalid partition
                loadClusterConfig();
            } else {
                return response;
            }
        }
    }

    private List<Node> getNodesInPartitionByKey(String key) {
        return ringEntries.getNodesIdByKey(key).stream()
                .map(nodeId -> this.nodesById.get(nodeId))
                .toList();
    }

    private Response sendRequestToRandomNode(List<Node> nodes, Request request) {
        Set<String> seedNodeAddressTried = new HashSet<>();

        while(seedNodeAddressTried.size() < nodes.size()) {
            Node seedNode = getRandomNode(seedNodeAddressTried, nodes)
                    .orElseThrow(() -> new RuntimeException("No node available to send request"));

            Try<Response> response = Try.of(() -> seedNode.sendRequest(request));

            if(response.isSuccess()){
                return response.get();
            }
        }

        throw new RuntimeException("No node available to send request");
    }

    private Optional<Node> getRandomNode(Set<String> addressSeedNodesTried, List<Node> nodesToTakeRandomNode) {
        int indexSeedNode = (int) (Math.random() * nodesToTakeRandomNode.size());;
        Node seedNodeSelected = nodesToTakeRandomNode.get(indexSeedNode);

        while(addressSeedNodesTried.size() < nodesToTakeRandomNode.size()){
            if(!addressSeedNodesTried.contains(seedNodeSelected.getAddress())){
                addressSeedNodesTried.add(seedNodeSelected.getAddress());
                return Optional.of(seedNodeSelected);
            }
        }

        return Optional.empty();
    }

    private synchronized void loadClusterConfig() {
        if (clusterConfig != null) { //Double check concurrency
            Response responseClusterConfig = sendRequestToRandomNode(seedNodes, Request.builder()
                    .authentication(AuthenticationRequest.builder().authApiKey(authUserKey).build())
                    .operationRequest(OperationRequest.builder().operator(Operator.GET_CLUSTER_CONFIG).build())
                    .build());
            clusterConfig = deserializeGetConfigRequest(responseClusterConfig);

            allNodes = clusterConfig.clusterNodes().stream()
                    .map(node -> new Node(node.address(), node.nodeId()))
                    .collect(Collectors.toList());

            ringEntries = RingEntries.fromClusterConfig(clusterConfig);
        }
    }

    private ClusterConfig deserializeGetConfigRequest(Response response) {
        ByteBuffer bytesClusterConfigResponse = ByteBuffer.wrap(response.getResponse().getBytes())
                .order(ByteOrder.BIG_ENDIAN);

        int nodesPerPartition = bytesClusterConfigResponse.getInt(0);
        int maxNodesInPartition = bytesClusterConfigResponse.get(4);
        int nNodes = bytesClusterConfigResponse.get(8);
        int offset = 12;

        List<ClusterConfigNode> clusterNodes = new ArrayList<>(nNodes);
        for(int i = 0; i < nNodes; i++){
            short nodeId = bytesClusterConfigResponse.getShort(offset);
            int reqLength = bytesClusterConfigResponse.getShort(offset + 2);
            byte[] addressBytes = new byte[reqLength];
            bytesClusterConfigResponse.get(addressBytes, offset + 6, reqLength);
            String addressString = new String(addressBytes, StandardCharsets.US_ASCII);
            offset += reqLength + 2 + 4;

            clusterNodes.add(new ClusterConfigNode(nodeId, addressString));
        }

        List<ClusterConfigRingEntry> ringEntries = new ArrayList<>();
        if(nodesPerPartition > 0 && maxNodesInPartition > 0) { //Maybe partitions disabled
            for(int i = 0; i < nNodes; i++) {
                short nodeId = bytesClusterConfigResponse.getShort(offset);
                int ringPosition = bytesClusterConfigResponse.getInt(offset + 2);
                ringEntries.add(new ClusterConfigRingEntry(nodeId, ringPosition));

                offset += 4 + 2;
            }
        }

        return new ClusterConfig(nodesPerPartition, maxNodesInPartition, clusterNodes, ringEntries);
    }
}
