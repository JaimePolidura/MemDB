package es.memdb.stresstest;

public enum StressTestOperation {
    SET(1), GET(2), DELETE(3);

    public final int number;

    StressTestOperation(int number) {
        this.number = number;
    }
}
