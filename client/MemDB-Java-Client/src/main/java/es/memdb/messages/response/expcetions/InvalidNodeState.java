package es.memdb.messages.response.expcetions;

import es.memdb.messages.request.Request;

public final class InvalidNodeState extends MemDbException {
    public InvalidNodeState(String expcetionName, Request request) {
        super(expcetionName, request);
    }
}
