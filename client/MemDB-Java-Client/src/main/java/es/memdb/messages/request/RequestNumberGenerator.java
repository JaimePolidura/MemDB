package es.memdb.messages.request;

import java.util.concurrent.atomic.AtomicLong;

public final class RequestNumberGenerator {
    private final AtomicLong last = new AtomicLong(0);

    public long next() {
        return this.last.getAndIncrement();
    }
}
