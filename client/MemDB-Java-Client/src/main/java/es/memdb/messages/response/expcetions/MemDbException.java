package es.memdb.messages.response.expcetions;

import es.memdb.messages.request.Request;

public abstract class MemDbException extends RuntimeException {
    public MemDbException(String expcetionName, Request request) {
        super(String.format("Exception %s occurred while executing operation %s",
                expcetionName,
                request.getOperationRequest().getOperator().name()));
    }
}
