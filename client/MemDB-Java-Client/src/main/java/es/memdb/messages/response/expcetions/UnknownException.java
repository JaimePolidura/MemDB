package es.memdb.messages.response.expcetions;

import es.memdb.messages.request.Request;

public final class UnknownException extends MemDbException {
    public UnknownException(String expcetionName, Request request) {
        super(expcetionName, request);
    }
}
