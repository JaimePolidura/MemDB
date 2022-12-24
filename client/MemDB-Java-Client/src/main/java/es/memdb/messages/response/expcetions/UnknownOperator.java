package es.memdb.messages.response.expcetions;

import es.memdb.messages.request.Request;

public final class UnknownOperator extends MemDbException {
    public UnknownOperator(String message, Request request) {
        super(request);
    }
}
