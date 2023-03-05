package es.memdb.messages.request;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Getter;

@Builder
@AllArgsConstructor
public final class Request {
    @Getter private final int requestNumber;
    @Getter private final AuthenticationRequest authentication;
    @Getter private final OperationRequest operationRequest;

    @Override
    public String toString() {
        return "Request{" +
                "requestNumber=" + requestNumber +
                ", operator=" + operationRequest.getOperator() +
                '}';
    }
}
