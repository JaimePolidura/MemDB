package es.memdb;

import es.memdb.connection.MemDbConnection;
import es.memdb.messages.request.AuthenticationRequest;
import es.memdb.messages.request.OperationRequest;
import es.memdb.messages.request.Request;
import es.memdb.messages.request.RequestSerializer;
import es.memdb.messages.response.Response;
import es.memdb.messages.response.ResponseDeserializer;
import es.memdb.messages.response.expcetions.MemDbException;
import es.memdb.messages.response.expcetions.MemDbExceptionsRegistry;
import lombok.RequiredArgsConstructor;
import lombok.SneakyThrows;

import java.util.List;

@RequiredArgsConstructor
public final class MemDb {
    private MemDbExceptionsRegistry memDbExceptionsRegistry = new MemDbExceptionsRegistry();
    private ResponseDeserializer responseDeserializer = new ResponseDeserializer();
    private RequestSerializer requestSerializer = new RequestSerializer();

    private final MemDbConnection memDbConnection;
    private final String authKey;

    public String get(String key) {
        return this.sendRequest(OperationRequest.builder()
                .operator(Operator.GET)
                .args(List.of(key)));
    }

    public String set(String key, String value) {
        return this.sendRequest(OperationRequest.builder()
                .operator(Operator.SET)
                .args(List.of(key, value)));
    }

    private String sendRequest(OperationRequest.OperationRequestBuilder operation) {
        Request request = Request.builder()
                .authentication(AuthenticationRequest.builder().authKey(this.authKey).build())
                .operationRequest(operation.build())
                .build();
        byte[] rawRequest = this.requestSerializer.serialize(request);

        this.memDbConnection.write(rawRequest);

        byte[] rawResponse = this.memDbConnection.read();
        Response response = this.responseDeserializer.deserialize(rawResponse);

        return response.isFailed() ?
                handleException(request, response) :
                response.getResponse();
    }

    @SneakyThrows
    private String handleException(Request request, Response response) {
        int errorCode = response.getErrorCode();
        Class<? extends MemDbException> exception = this.memDbExceptionsRegistry.findByCode(errorCode, request);
        boolean exceptionExpected = request.getOperationRequest().getOperator().isExpcetionExpected(exception);

        if(!exceptionExpected)
            throw (MemDbException) exception.getConstructors()[0].newInstance(exception.getName(), request);
        else
            return null;
    }
}
