package es.memdb.stresstest;

public enum StressTestOperation {
    SET(1, "SET"), GET(2, "GET"), DELETE(3, "DEL");

    public final int number;
    public final String alias;

    StressTestOperation(int number, String alias) {
        this.number = number;
        this.alias = alias;
    }
}
