package es.memdb;

import es.memdb.connection.MemDbConnection;
import es.memdb.messages.request.*;
import es.memdb.messages.response.Response;
import es.memdb.messages.response.ResponseDeserializer;
import es.memdb.messages.response.expcetions.MemDbException;
import es.memdb.messages.response.expcetions.MemDbExceptionsRegistry;
import es.memdb.utils.clock.LamportClock;
import lombok.RequiredArgsConstructor;
import lombok.SneakyThrows;

import java.util.List;
import java.util.function.Consumer;

@RequiredArgsConstructor
public final class MemDb {
    private MemDbExceptionsRegistry memDbExceptionsRegistry = new MemDbExceptionsRegistry();
    private RequestNumberGenerator requestNumberGenerator = new RequestNumberGenerator();
    private ResponseDeserializer responseDeserializer = new ResponseDeserializer();
    private RequestSerializer requestSerializer = new RequestSerializer();

    private final LamportClock clock = new LamportClock();
    private final MemDbConnection memDbConnection;
    private final String authKey;


    public String get(String key) {
        return this.sendRequest(OperationRequest.builder()
                .operator(Operator.GET)
                .args(List.of(key)));
    }

    public void get(String key, Consumer<String> callBack) {
        this.sendRequestAsync(callBack, OperationRequest.builder()
                .operator(Operator.GET)
                .args(List.of(key)));
    }

    public void delete(String key) {
        this.sendRequest(OperationRequest.builder()
                .operator(Operator.DELETE)
                .args(List.of(key)));
    }

    public void delete(String key, Runnable runnable) {
        this.sendRequestAsync(response -> runnable.run(), OperationRequest.builder()
                .operator(Operator.DELETE)
                .args(List.of(key)));
    }

    public void set(String key, String value) {
        this.sendRequest(OperationRequest.builder()
                .operator(Operator.SET)
                .args(List.of(key, value)));
    }

    public void set(String key, String value, Runnable callback) {
        this.sendRequestAsync(response -> callback.run(), OperationRequest.builder()
                .operator(Operator.SET)
                .args(List.of(key, value)));
    }

    private String sendRequest(OperationRequest.OperationRequestBuilder operation) {
        Request request = this.createRequestObject(operation);

        byte[] rawRequest = this.requestSerializer.serialize(request, this.clock.get());

        this.memDbConnection.write(rawRequest);

        byte[] rawResponse = this.memDbConnection.read(request.getRequestNumber());
        Response response = this.responseDeserializer.deserialize(rawResponse);

        return response.isFailed() ?
                handleException(request, response) :
                response.getResponse();
    }

    private void sendRequestAsync(Consumer<String> result, OperationRequest.OperationRequestBuilder operation) {
        Request request = this.createRequestObject(operation);
        byte[] rawRequest = this.requestSerializer.serialize(request, this.clock.get());

        this.memDbConnection.write(rawRequest, rawResponse -> {
            Response response = this.responseDeserializer.deserialize(Utils.wrapperToPrimitive(rawResponse));

            result.accept(response.isFailed() ?
                            handleException(request, response) :
                            response.getResponse()
            );
        });
    }

    private Request createRequestObject(OperationRequest.OperationRequestBuilder operation) {
        long requestNumber = this.requestNumberGenerator.next();

        return Request.builder()
                .requestNumber(requestNumber)
                .authentication(AuthenticationRequest.builder().authKey(authKey).build())
                .operationRequest(operation.build())
                .build();
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
