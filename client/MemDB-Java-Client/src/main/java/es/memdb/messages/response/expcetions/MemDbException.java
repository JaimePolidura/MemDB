package es.memdb.messages.response.expcetions;

import es.memdb.messages.request.Request;

public abstract class MemDbException extends RuntimeException {
    public MemDbException(String exceptionName, Request request) {
        super(String.format("Exception with code %s occurred while executing operation %s",
                exceptionName,
                request.getOperationRequest().getOperator().name()));
    }
}
