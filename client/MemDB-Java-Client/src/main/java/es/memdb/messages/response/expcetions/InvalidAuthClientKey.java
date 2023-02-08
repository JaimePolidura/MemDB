package es.memdb.messages.response.expcetions;

import es.memdb.messages.request.Request;

public final class InvalidAuthClientKey extends MemDbException {
    public InvalidAuthClientKey(String expcetionName, Request request) {
        super(expcetionName, request);
    }
}
