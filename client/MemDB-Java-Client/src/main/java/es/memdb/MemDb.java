package es.memdb;

import es.memdb.connection.MemDbConnection;
import es.memdb.messages.request.*;
import es.memdb.messages.response.MultiResponses;
import es.memdb.messages.response.Response;
import es.memdb.messages.response.ResponseDeserializer;
import es.memdb.messages.response.expcetions.MemDbException;
import es.memdb.messages.response.expcetions.MemDbExceptionsRegistry;
import es.memdb.utils.LamportClock;
import lombok.RequiredArgsConstructor;
import lombok.SneakyThrows;

import java.util.Arrays;
import java.util.List;

@RequiredArgsConstructor
public final class MemDb {
    private MemDbExceptionsRegistry memDbExceptionsRegistry = new MemDbExceptionsRegistry();
    private RequestNumberGenerator requestNumberGenerator = new RequestNumberGenerator();
    private ResponseDeserializer responseDeserializer = new ResponseDeserializer();
    private RequestSerializer requestSerializer = new RequestSerializer();

    private final LamportClock clock = new LamportClock(0L, (short) 0);
    private final MemDbConnection memDbConnection;
    private final String authApiKey;
    private final boolean isCluster;

    public String get(String key) {
        return sendRequest(OperationRequest.builder()
                .operator(Operator.GET)
                .args(List.of(key)));
    }

    public MultiResponses quorumGet(String key) {
        Request request = this.createRequestObject(OperationRequest.builder()
                .operator(Operator.GET)
                .args(List.of(key)));

        if(!isCluster){
            return MultiResponses.fromSingleResponse(sendRequestAndReceiveResponse(request));
        } else {
            return sendMultipleRequest(request);
        }
    }

    public void delete(String key) {
        sendRequest(OperationRequest.builder()
                .operator(Operator.DELETE)
                .args(List.of(key)));
    }

    public void set(String key, String value) {
        sendRequest(OperationRequest.builder()
                .operator(Operator.SET)
                .args(List.of(key, value)));
    }

    public boolean cas(String key, String expected, String value) {
        return sendRequest(OperationRequest.builder()
                .operator(Operator.CAS)
                .args(List.of(key, expected, value))) != null;
    }

    private MultiResponses sendMultipleRequest(Request request) {
        int nNodes = /* TODO **/ 0;
        MultiResponses multiResponses = MultiResponses.fromCluster(nNodes);
        MultiResponses.MultipleResponseNotifier notifier = new MultiResponses.MultipleResponseNotifier(multiResponses);

        for (int i = 0; i < nNodes; i++) {
            notifier.addResponse(/** TOOD */ null);
            //Send req
        }

        return multiResponses;
    }

    private String sendRequest(OperationRequest.OperationRequestBuilder operation) {
        Request request = createRequestObject(operation);
        Response response = sendRequestAndReceiveResponse(request);

        return handleResponse(request, response);
    }

    private Response sendRequestAndReceiveResponse(Request request) {
        byte[] rawRequest = requestSerializer.serialize(request, clock.get());

        memDbConnection.write(rawRequest, request.getRequestNumber());

        byte[] rawResponse = memDbConnection.read(request.getRequestNumber());

        Response response = responseDeserializer.deserialize(rawResponse);

        clock.update(response.getTimestamp().getCounter().get());

        return response;
    }

    private String handleResponse(Request request, Response response) {
        return response.isFailed() ?
                handleException(request, response) :
                response.getResponse();
    }

    private Request createRequestObject(OperationRequest.OperationRequestBuilder operation) {
        int requestNumber = requestNumberGenerator.next();

        return Request.builder()
                .requestNumber(requestNumber)
                .authentication(AuthenticationRequest.builder().authApiKey(authApiKey).build())
                .operationRequest(operation.build())
                .build();
    }

    @SneakyThrows
    private String handleException(Request request, Response response) {
        int errorCode = response.getErrorCode();
        Class<? extends MemDbException> exception = memDbExceptionsRegistry.findByCode(errorCode, request);
        boolean exceptionExpected = request.getOperationRequest().getOperator().isExpcetionExpected(exception);

        if(!exceptionExpected)
            throw (MemDbException) exception.getConstructors()[0].newInstance(exception.getName(), request);
        else
            return null;
    }
}
