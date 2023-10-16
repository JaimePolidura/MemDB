package es.memdb.messages.response.expcetions;

import es.memdb.messages.request.Request;

public final class InvalidPartition extends MemDbException {
    public InvalidPartition(String expcetionName, Request request) {
        super(expcetionName, request);
    }
}
