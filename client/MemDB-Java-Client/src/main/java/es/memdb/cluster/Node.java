package es.memdb.cluster;

import es.memdb.connection.MemDbConnection;
import es.memdb.connection.NoClusterMemDbConnection;
import es.memdb.connection.ResponseReader;
import es.memdb.messages.request.Request;
import es.memdb.messages.request.RequestSerializer;
import es.memdb.messages.response.Response;
import es.memdb.messages.response.ResponseDeserializer;
import es.memdb.utils.LamportClock;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Getter;
import lombok.NoArgsConstructor;

import java.io.IOException;
import java.util.Objects;

public final class Node {
    @Getter private int nodeId;
    @Getter private String address;
    @Getter private MemDbConnection connection;

    private final ResponseDeserializer responseDeserializer = new ResponseDeserializer();
    private final RequestSerializer requestSerializer = new RequestSerializer();

    public Node(String address) {
        this(address, 0);
    }

    public Node(String address, int nodeId) {
        this.address = address;
        this.nodeId = nodeId;
    }

    public Response sendRequest(Request request) {
        if(connection.isClosed()){
            connect();
        }

        byte[] responseBytes = requestSerializer.serialize(request, request.getOperationRequest().getTimestamp());
        return responseDeserializer.deserialize(responseBytes);
    }

    public void connect() {
        try{
            String host = address.split(":")[0];
            int port = Integer.parseInt(address.split(":")[1]);
            connection = new NoClusterMemDbConnection(host, port);
            connection.connect();
        }catch (Exception e) {
            throw new RuntimeException(e.getMessage());
        }
    }

    public int getPort() {
        return Integer.parseInt(this.address.split(":")[1]);
    }

    public String getIP() {
        return this.address.split(":")[0];
    }
}
