package es.memdb.utils.clock;

import java.util.concurrent.atomic.AtomicLong;

public final class LamportClock {
    private final AtomicLong count;

    public LamportClock(long initialValue) {
        this.count = new AtomicLong(initialValue);
    }

    public long update(long other) {
        long actualCounter;
        long newCounter;

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
