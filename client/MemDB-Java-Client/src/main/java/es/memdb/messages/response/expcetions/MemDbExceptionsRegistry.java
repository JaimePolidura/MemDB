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
        AUTH_KEY_INVALID(0, InvalidAuthApiKey.class),
        UNKNOWN_KEY(1, UnknownKey.class),
        UNKNOWN_OPERATOR(2, UnknownOperator.class),
        ALREADY_REPLICATED(3, AlreadyReplicated.class),
        NOT_AUTHORIZED(4, NotAuthorized.class),
        INVALID_NODE_STATE(5, InvalidNodeState.class),
        INVALID_PARTITION(6, InvalidPartition.class),
        INVALID_CALL(7, InvalidCall.class),
        CAS_FAILED(10, CasFailed.class);

        public final int code;
        public final Class<? extends MemDbException> clazz;

        Exceptions(int code, Class<? extends MemDbException> clazz) {
            this.code = code;
            this.clazz = clazz;
        }
    }
}
