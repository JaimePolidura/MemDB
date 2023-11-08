package es.memdb.utils;

import lombok.Getter;

import java.util.concurrent.atomic.AtomicLong;

public final class LamportClock implements Comparable<LamportClock> {
    @Getter private final AtomicLong counter;
    @Getter private final short nodeId;

    public LamportClock(long initialValue, short nodeId) {
        this.counter = new AtomicLong(initialValue);
        this.nodeId = nodeId;
    }

    public long update(long other) {
        long actualCounter;
        long newCounter;

        do {
            actualCounter = this.counter.get();

            newCounter = Math.max(actualCounter, other);
        }while (this.counter.compareAndExchange(actualCounter, newCounter) != actualCounter);

        return newCounter;
    }

    public long get() {
        return this.counter.get();
    }

    @Override
    public int compareTo(LamportClock o) {
        if(o.counter.get() != this.counter.get()) {
            return (int) (this.counter.get() - o.counter.get());
        } else {
            return this.nodeId - o.nodeId;
        }
    }
}
