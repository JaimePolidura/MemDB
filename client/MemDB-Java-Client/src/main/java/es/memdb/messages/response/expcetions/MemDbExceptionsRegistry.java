package es.memdb.messages.response.expcetions;

import es.memdb.messages.request.Request;

import java.util.Arrays;

public final class MemDbExceptionsRegistry {
    public Class<? extends MemDbException> findByCode(int code, Request request) {
        return Arrays.stream(Exceptions.values())
                .filter(it -> it.code == code)
                .findFirst()
                .map(it -> it.clazz)
                .orElseThrow(() -> new UnknownException("unknown exception with code: " + code, request));
    }

    private enum Exceptions {
        AUTH_KEY_INVALID(0x00, InvalidAuthApiKey.class),
        UNKNOWN_KEY(0x01, UnknownKey.class),
        UNKNOWN_OPERATOR(0x02, UnknownOperator.class),
        ALREADY_REPLICATED(0x03, AlreadyReplicated.class),
        NOT_AUTHORIZED(0x04, NotAuthorized.class),
        INVALID_NODE_STATE(0x05, InvalidNodeState.class),
        INVALID_PARTITION(0x06, InvalidPartition.class),
        INVALID_CALL(0x07, InvalidCall.class);

        public final int code;
        public final Class<? extends MemDbException> clazz;

        Exceptions(int code, Class<? extends MemDbException> clazz) {
            this.code = code;
            this.clazz = clazz;
        }
    }
}
