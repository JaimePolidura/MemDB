package es.memdb.cluster;

import com.fasterxml.jackson.databind.ObjectMapper;
import es.memdb.cluster.messages.GetAllNodesResponse;
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
    private final String authClusterKey;
    private String lastToken;

    public ClusterManager(List<String> addresses, String authClusterKey) {
        this.addresses = addresses;
        this.authClusterKey = authClusterKey;
        this.lastToken = authenticate();
    }

    @SneakyThrows
    public List<Node> getAllNodes() {
        HttpRequest getAllNodesRequest = HttpRequest.newBuilder()
                .uri(URI.create(getRandomClusterManagerAddress()))
                .setHeader("Authentication", "Bearer " + this.lastToken)
                .POST(HttpRequest.BodyPublishers.ofString("{\"authKey\": \""+this.authClusterKey+"\"}"))
                .build();

        HttpResponse<String> response = this.httpClient.send(getAllNodesRequest, HttpResponse.BodyHandlers.ofString());

        if(response.statusCode() == 403) {
            this.lastToken = authenticate();
            return this.getAllNodes();
        }else{
            return this.objectMapper.readValue(response.body(), GetAllNodesResponse.class).getNodes();
        }
    }

    @SneakyThrows
    private String authenticate() {
        HttpRequest loginRequest = HttpRequest.newBuilder()
                .uri(URI.create(getRandomClusterManagerAddress()))
                .POST(HttpRequest.BodyPublishers.ofString("{\"authKey\": \""+this.authClusterKey+"\"}"))
                .build();

        HttpResponse<String> response = this.httpClient.send(loginRequest, HttpResponse.BodyHandlers.ofString());
        LoginResponse responseLogin = this.objectMapper.readValue(response.body(), LoginResponse.class);

        return responseLogin.getToken();
    }

    private String getRandomClusterManagerAddress() {
        return this.addresses.get((int) (Math.random() * this.addresses.size()));
    }
}
