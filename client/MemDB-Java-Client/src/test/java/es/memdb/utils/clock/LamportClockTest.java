package es.memdb.utils.clock;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;

public class LamportClockTest  {
    private LamportClock lamportClockToTest;

    @Before
    public void setup() {
        this.lamportClockToTest = new LamportClock();
    }

    @Test
    public void initializeWithCountZero() {
        Assert.assertEquals(0, this.lamportClockToTest.get());
    }

    @Test
    public void shouldUpdate() {
        this.lamportClockToTest.update(1);
        this.lamportClockToTest.update(2);
        Assert.assertEquals(2, this.lamportClockToTest.get());

        this.lamportClockToTest.update(0);
        Assert.assertEquals(2, this.lamportClockToTest.get());
    }
}
