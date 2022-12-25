package es.memdb.messages.response.expcetions;

import es.memdb.messages.request.Request;

public final class UnknownKey extends MemDbException {
    public UnknownKey(String expcetionName, Request request) {
        super(expcetionName, request);
    }
}
