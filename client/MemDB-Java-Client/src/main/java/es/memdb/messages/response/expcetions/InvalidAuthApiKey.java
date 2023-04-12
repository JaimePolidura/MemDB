package es.memdb.messages.response.expcetions;

import es.memdb.messages.request.Request;

public final class InvalidAuthApiKey extends MemDbException {
    public InvalidAuthApiKey(String expcetionName, Request request) {
        super(expcetionName, request);
    }
}
