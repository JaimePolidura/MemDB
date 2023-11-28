package es.memdb.cluster;

import es.memdb.Operator;
import es.memdb.Utils;
import es.memdb.connection.MemDbConnection;
import es.memdb.messages.request.AuthenticationRequest;
import es.memdb.messages.request.OperationRequest;
import es.memdb.messages.request.Request;
import es.memdb.messages.response.Response;
import io.vavr.control.Try;
import lombok.AllArgsConstructor;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.StandardCharsets;
import java.util.*;
import java.util.stream.Collectors;

@AllArgsConstructor
public final class ClusterMemDbConnection implements MemDbConnection {
    private final List<Node> otherNodes;
    private final List<Node> seedNodes;
    private final String authUserKey;

    private ClusterConfig clusterConfig;

    public ClusterMemDbConnection(List<String> seedNodesAddress, String authUserKey) {
        List<Node> seedNodes = createSeedNodes(seedNodesAddress);

        this.otherNodes = new ArrayList<>(seedNodes);
        this.authUserKey = authUserKey;
        this.seedNodes = seedNodes;
    }

    @Override
    public void connect() throws IOException {
        if(clusterConfig != null){
            clusterConfig = getClusterConfigFromSeedNodes();
        }
    }

    @Override
    public Response send(Request request) {
        boolean hasKey = !request.getOperationRequest().getArgs().isEmpty();

        Utils.rethrowNoChecked(this::connect);

        if(hasKey){

        }

        return new byte[0];
    }

    @Override
    public boolean isClosed() {
        return false;
    }

    @Override
    public void close() throws Exception {

    }

    private List<Node> createSeedNodes(List<String> seedNodeAddresses) {
        return seedNodeAddresses.stream()
                .map(Node::new)
                .collect(Collectors.toList());
    }

    private ClusterConfig getClusterConfigFromSeedNodes() {
        Set<String> seedNodeAddressTried = new HashSet<>();

        while(seedNodeAddressTried.size() < seedNodes.size()){
            Node seedNode = getRandomNode(seedNodeAddressTried, seedNodes)
                    .orElseThrow(() -> new RuntimeException("No seed node available to send getClusterConfig request"));

            Try<Response> response = Try.of(() ->seedNode.sendRequest(Request.builder()
                    .authentication(AuthenticationRequest.builder().authApiKey(authUserKey).build())
                    .operationRequest(OperationRequest.builder().operator(Operator.GET_CLUSTER_CONFIG).build())
                    .build()));

            if(response.isSuccess()){
                return deserializeGetConfigRequest(response.get());
            }
        }

        throw new RuntimeException("No seed node available to send getClusterConfig request");
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

        List<ClusterConfigRingEntries> ringEntries = new ArrayList<>();
        if(nodesPerPartition > 0 && maxNodesInPartition > 0) { //Maybe partitions disabled
            for(int i = 0; i < nNodes; i++) {
                short nodeId = bytesClusterConfigResponse.getShort(offset);
                int ringPosition = bytesClusterConfigResponse.getInt(offset + 2);
                ringEntries.add(new ClusterConfigRingEntries(nodeId, ringPosition));

                offset += 4 + 2;
            }
        }

        return new ClusterConfig(nodesPerPartition, maxNodesInPartition, clusterNodes, ringEntries);
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
}
