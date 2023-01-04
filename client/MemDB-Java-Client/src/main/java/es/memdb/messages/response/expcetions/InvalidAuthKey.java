package es.memdb.messages.response.expcetions;

import es.memdb.messages.request.Request;

public final class InvalidAuthKey extends MemDbException {
    public InvalidAuthKey(String expcetionName, Request request) {
        super(expcetionName, request);
    }
}
