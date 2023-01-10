package es.memdb.stresstest;

import es.memdb.stresstest.executer.StressTestOperationExecuter;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Random;

public final class StressTestThread extends Thread {
    private final StressTestOperationExecuter stressTestOperationExecuter;
    private final List<StressTestResult> results;
    private final int numberOperations;

    public StressTestThread(int numberOperations, StressTestOperationExecuter stressTestExecuter) {
        this.numberOperations = numberOperations;
        this.stressTestOperationExecuter = stressTestExecuter;
        this.results = new ArrayList<>(this.numberOperations);
    }

    @Override
    public void run() {
        Random randomGenerator = new Random();

        for(int i = 0; i < this.numberOperations; i++){
            int operatorNumber = randomGenerator.nextInt(0, 2) + 1;

            StressTestOperation operatorPerform = Arrays.stream(StressTestOperation.values())
                    .filter(operation -> operation.number == operatorNumber)
                    .findFirst()
                    .get();

            long result = this.execute(operatorPerform);
            this.results.add(new StressTestResult(result, operatorPerform));
        }
    }

    private long execute(StressTestOperation operatorPerform) {
        long a = System.currentTimeMillis();
        this.stressTestOperationExecuter.execute(operatorPerform);
        long b = System.currentTimeMillis();

        return b - a;
    }

    public List<StressTestResult> getResults() {
        return results;
    }
}
