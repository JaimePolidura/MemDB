package es.memdb;

import es.memdb.messages.response.expcetions.MemDbException;
import es.memdb.messages.response.expcetions.UnknownKey;

import java.util.Collections;
import java.util.List;

public enum Operator {
    SET(1, Collections.EMPTY_LIST),
    GET(2, List.of(UnknownKey.class));

    public final List<Class<? extends MemDbException>> expectedExceptions;
    public final int operatorNumber;

    public boolean isExpcetionExpected(Class<? extends MemDbException> exceptionClass) {
        return this.expectedExceptions.stream()
                .anyMatch(it -> it == exceptionClass);
    }

    Operator(int operatorNumber, List<Class<? extends MemDbException>> expectedExceptions) {
        this.expectedExceptions = expectedExceptions;
        this.operatorNumber = operatorNumber;
    }
}