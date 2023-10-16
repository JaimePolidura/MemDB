package es.memdb.messages.response.expcetions;

import es.memdb.messages.request.Request;

public final class InvalidCall extends MemDbException {
    public InvalidCall(String exceptionName, Request request) {
        super(exceptionName, request);
    }
}
