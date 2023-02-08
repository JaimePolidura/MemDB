package es.memdb.messages.response.expcetions;

import es.memdb.messages.request.Request;

public final class NotAuthorized extends MemDbException {
    public NotAuthorized(String expcetionName, Request request) {
        super(expcetionName, request);
    }
}
