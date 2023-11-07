package es.memdb.utils;

import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

public final class CountUpLatch {
    private final AtomicInteger count = new AtomicInteger(0);

    public int countUp() {
        int last = count.getAndIncrement();
        super.notifyAll();
        return last;
    }

    public boolean awaitMin(int value, long time, TimeUnit timeUnit) {
        try {
            while(count.get() < value){
                super.wait(TimeUnit.MICROSECONDS.convert(time, timeUnit));
            }

            return true;
        } catch (InterruptedException e) {
            return false;
        }
    }
}
