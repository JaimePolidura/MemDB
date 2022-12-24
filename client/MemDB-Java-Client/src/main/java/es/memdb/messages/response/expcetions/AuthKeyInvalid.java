package es.memdb.messages.response.expcetions;

import es.memdb.messages.request.Request;

public final class AuthKeyInvalid extends MemDbException {
    public AuthKeyInvalid(String message, Request request) {
        super(request);
    }
}
