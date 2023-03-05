package es.memdb.messages.request;

import java.util.concurrent.atomic.AtomicInteger;

public final class RequestNumberGenerator {
    private final AtomicInteger last = new AtomicInteger(0);

    public int next() {
        return this.last.getAndIncrement();
    }
}
