package es.memdb.cluster;

import com.fasterxml.jackson.databind.ObjectMapper;
import es.memdb.cluster.messages.LoginResponse;
import lombok.SneakyThrows;

import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.util.List;

public final class ClusterManager {
    private final HttpClient httpClient = HttpClient.newHttpClient();
    private final ObjectMapper objectMapper = new ObjectMapper();

    private final List<String> addresses;
    private final String authApiKey;
    private String lastToken;

    public ClusterManager(List<String> addresses, String authApiKey) {
        this.addresses = addresses;
        this.authApiKey = authApiKey;
        this.lastToken = authenticate();
    }

    @SneakyThrows
    public List<Node> getAllNodes() {
//        HttpRequest getAllNodesRequest = HttpRequest.newBuilder()
//                .uri(URI.create(getRandomClusterManagerAddress() + "/api/nodes/all"))
//                .setHeader("Authorization", "Bearer " + this.lastToken)
//                .GET()
//                .build();
//
//        HttpResponse<String> response = this.httpClient.send(getAllNodesRequest, HttpResponse.BodyHandlers.ofString());
//
//        if(response.statusCode() == 401 || response.statusCode() == 403) {
//            this.lastToken = authenticate();
//            return this.getAllNodes();
//        }else{
//            return this.objectMapper.readValue(response.body(), GetAllNodesResponse.class).getNodes();
//        }

        return List.of(
                new Node(1, NodeState.RUNNING, "127.0.0.1:10000"),
                new Node(2, NodeState.RUNNING, "127.0.0.1:10001")
        );
    }

    @SneakyThrows
    private String authenticate() {
        HttpRequest loginRequest = HttpRequest.newBuilder()
                .uri(URI.create(getRandomClusterManagerAddress() + "/login"))
                .setHeader("Content-Type", "application/json")
                .POST(HttpRequest.BodyPublishers.ofString("{\"authKey\": \""+this.authApiKey +"\"}"))
                .build();

        HttpResponse<String> response = this.httpClient.send(loginRequest, HttpResponse.BodyHandlers.ofString());
        LoginResponse responseLogin = this.objectMapper.readValue(response.body(), LoginResponse.class);

        return responseLogin.getToken();
    }

    private String getRandomClusterManagerAddress() {
        return this.addresses.get((int) (Math.random() * this.addresses.size()));
    }
}
