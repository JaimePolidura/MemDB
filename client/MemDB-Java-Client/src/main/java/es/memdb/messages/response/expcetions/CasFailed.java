package es.memdb.messages.response.expcetions;

import es.memdb.messages.request.Request;

public final class CasFailed extends MemDbException {
    public CasFailed(String exceptionName, Request request) {
        super(exceptionName, request);
    }
}
