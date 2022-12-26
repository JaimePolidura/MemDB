package es.memdb.messages.response.expcetions;

import es.memdb.messages.request.Request;

import java.util.Arrays;

public final class MemDbExceptionsRegistry {
    public Class<? extends MemDbException> findByCode(int code, Request request) {
        return Arrays.stream(Exceptions.values())
                .filter(it -> it.code == code)
                .findFirst()
                .map(it -> it.clazz)
                .orElseThrow(() -> new UnknownException("unknown exception ", request));
    }

    private enum Exceptions {
        AUTH_KEY_INVALID(0x00, AuthKeyInvalid.class),
        UNKNOWN_KEY(0x01, UnknownKey.class),
        UNKNOWN_OPERATOR(0x02, UnknownOperator.class);

        public final int code;
        public final Class<? extends MemDbException> clazz;

        Exceptions(int code, Class<? extends MemDbException> clazz) {
            this.code = code;
            this.clazz = clazz;
        }
    }
}
