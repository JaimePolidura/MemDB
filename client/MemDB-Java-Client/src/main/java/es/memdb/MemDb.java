package es.memdb;

import es.memdb.cluster.ClusterMemDbConnection;
import es.memdb.connection.MemDbConnection;
import es.memdb.messages.request.*;
import es.memdb.messages.response.MultiResponses;
import es.memdb.messages.response.Response;
import es.memdb.messages.response.expcetions.MemDbException;
import es.memdb.messages.response.expcetions.MemDbExceptionsRegistry;
import es.memdb.utils.LamportClock;
import lombok.RequiredArgsConstructor;
import lombok.SneakyThrows;

import java.util.List;

@RequiredArgsConstructor
public final class MemDb {
    private MemDbExceptionsRegistry memDbExceptionsRegistry = new MemDbExceptionsRegistry();
    private RequestNumberGenerator requestNumberGenerator = new RequestNumberGenerator();

    private final LamportClock clock = new LamportClock(0L, (short) 0);
    private final MemDbConnection memDbConnection;
    private final String authApiKey;
    private final boolean isCluster;

    public String get(String key) {
        return sendRequest(OperationRequest.builder()
                .operator(Operator.GET)
                .args(List.of(key)))
                .toString();
    }

    public long getCounter(String key) {
        return sendRequest(OperationRequest.builder()
                .operator(Operator.GET)
                .args(List.of(key)))
                .toLong();
    }

    public MultiResponses getQuorumCounter(String key) {
        Request request = createRequestObject(OperationRequest.builder()
                .operator(Operator.GET)
                .args(List.of(key)));

        return sendMultiRequest(request);
    }

    public boolean contains(String key) {
        return sendRequest(OperationRequest.builder()
                .operator(Operator.CONTAINS)
                .args(List.of(key))) != null;
    }

    public MultiResponses quorumContains(String key) {
        Request request = createRequestObject(OperationRequest.builder()
                .operator(Operator.CONTAINS)
                .args(List.of(key)));

        return sendMultiRequest(request);
    }

    public MultiResponses quorumGet(String key) {
        Request request = this.createRequestObject(OperationRequest.builder()
                .operator(Operator.GET)
                .args(List.of(key)));

        return sendMultiRequest(request);
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

    public void increment(String key) {
        sendRequest(OperationRequest.builder()
                .operator(Operator.UPDATE_COUNTER)
                .flag1(true)
                .args(List.of(key)));
    }

    public void decrement(String key) {
        sendRequest(OperationRequest.builder()
                .operator(Operator.UPDATE_COUNTER)
                .flag1(false)
                .args(List.of(key)));
    }

    public boolean cas(String key, String expected, String value) {
        return sendRequest(OperationRequest.builder()
                .operator(Operator.CAS)
                .args(List.of(key, expected, value))) != null;
    }

    private MultiResponses sendMultiRequest(Request request) {
        if(!isCluster){
            return MultiResponses.fromSingleResponse(sendRequestAndReceiveResponse(request));
        } else {
            return sendMultipleRequest(request);
        }
    }

    private MultiResponses sendMultipleRequest(Request request) {
        ClusterMemDbConnection connectionCluster = (ClusterMemDbConnection) this.memDbConnection;
        return connectionCluster.sendMultipleRequest(request);
    }

    private Response sendRequest(OperationRequest.OperationRequestBuilder operation) {
        Request request = createRequestObject(operation);
        Response response = sendRequestAndReceiveResponse(request);

        return handleResponse(request, response);
    }

    private Response sendRequestAndReceiveResponse(Request request) {
        Response response = memDbConnection.send(request);

        clock.update(response.getTimestamp().getCounter().get());

        return response;
    }

    private Response handleResponse(Request request, Response response) {
        return response.isFailed() ?
                handleException(request, response) :
                response;
    }

    private Request createRequestObject(OperationRequest.OperationRequestBuilder operation) {
        int requestNumber = requestNumberGenerator.next();

        return Request.builder()
                .requestNumber(requestNumber)
                .authentication(AuthenticationRequest.builder().authApiKey(authApiKey).build())
                .operationRequest(operation
                        .timestamp(clock.get())
                        .build())
                .build();
    }

    @SneakyThrows
    private Response handleException(Request request, Response response) {
        int errorCode = response.getErrorCode();
        Class<? extends MemDbException> exception = memDbExceptionsRegistry.findByCode(errorCode, request);
        boolean exceptionExpected = request.getOperationRequest().getOperator().isExceptionExpected(exception);

        if(!exceptionExpected)
            throw (MemDbException) exception.getConstructors()[0].newInstance(exception.getName(), request);
        else
            return response.withResponse(new byte[]{0, 0, 0, 0});
    }
}
