package es.memdb;

import es.memdb.messages.response.expcetions.CasFailed;
import es.memdb.messages.response.expcetions.MemDbException;
import es.memdb.messages.response.expcetions.UnknownKey;

import java.util.Collections;
import java.util.List;

public enum Operator {
    SET(1, List.of(UnknownKey.class)),
    GET(2, List.of(UnknownKey.class)),
    DELETE(3, List.of(UnknownKey.class)),
    CAS(9, List.of(CasFailed.class)),
    GET_CLUSTER_CONFIG(12, Collections.EMPTY_LIST),
    CONTAINS(17, List.of(UnknownKey.class));

    public final List<Class<? extends MemDbException>> expectedExceptions;
    public final int operatorNumber;

    public boolean isExceptionExpected(Class<? extends MemDbException> exceptionClass) {
        return this.expectedExceptions.stream()
                .anyMatch(it -> it == exceptionClass);
    }

    Operator(int operatorNumber, List<Class<? extends MemDbException>> expectedExceptions) {
        this.expectedExceptions = expectedExceptions;
        this.operatorNumber = operatorNumber;
    }
}
