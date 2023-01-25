package es.memdb.utils.clock;

import java.util.concurrent.atomic.AtomicLong;

public final class LamportClock {
    private final AtomicLong count;

    public LamportClock() {
        this.count = new AtomicLong(0L);
    }

    public long update(long other) {
        long actualCounter = this.count.get();
        long newCounter = Math.max(actualCounter, other);

        do {
            actualCounter = this.count.get();
            newCounter = Math.max(actualCounter, other);
        }while (this.count.compareAndExchange(actualCounter, newCounter) != actualCounter);

        return newCounter;
    }

    public long get() {
        return this.count.get();
    }
}
