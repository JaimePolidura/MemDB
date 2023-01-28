package es.memdb.messages.response.expcetions;

import es.memdb.messages.request.Request;

public final class AlreadyReplicated extends MemDbException {
    public AlreadyReplicated(String expcetionName, Request request) {
        super(expcetionName, request);
    }
}
