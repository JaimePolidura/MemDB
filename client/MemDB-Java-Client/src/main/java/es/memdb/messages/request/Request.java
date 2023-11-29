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

    public boolean hasOneArg() {
        return !operationRequest.getArgs().isEmpty();
    }

    public String getFirstArg() {
        return this.operationRequest.getArgs().get(0);
    }

    @Override
    public String toString() {
        return "Request{" +
                "requestNumber=" + requestNumber +
                ", operator=" + operationRequest.getOperator() +
                '}';
    }
}
