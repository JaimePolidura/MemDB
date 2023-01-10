package es.memdb;

import es.memdb.connection.MemDbConnections;
import es.memdb.stresstest.StressTestOperation;
import es.memdb.stresstest.StressTestResult;
import es.memdb.stresstest.StressTestRunner;
import es.memdb.stresstest.executer.MemDbStressTestOperationExecuter;
import es.memdb.stresstest.executer.StressTestOperationExecuter;
import lombok.SneakyThrows;

import java.util.List;
import java.util.Map;
import java.util.function.Supplier;
import java.util.stream.Collectors;

public final class MemDbStressTest {
    @SneakyThrows
    public static void main(String[] args) {
        StressTestRunner memDbStressTestRunner = new StressTestRunner(
                1000, Runtime.getRuntime().availableProcessors() * 8
        );

        runAndPrintAverage(memDbStressTestRunner, memDbExecutorProvider());
    }

    private static void runAndPrintAverage(StressTestRunner memDbStressTestRunner, Supplier<StressTestOperationExecuter> memDbExecutorProvider) {
        List<StressTestResult> results =  memDbStressTestRunner.run(memDbExecutorProvider);

        Map<StressTestOperation, List<StressTestResult>> groupedByOperator = results.stream()
                .collect(Collectors.groupingBy(StressTestResult::operator));

        System.out.println("---------------------- FINISHED ----------------------");
        for (StressTestOperation operator : groupedByOperator.keySet()) {
            double average = groupedByOperator.get(operator).stream()
                    .mapToLong(StressTestResult::time)
                    .average()
                    .getAsDouble();

            System.out.println(operator.toString() + ": " + average);
        }
    }

    @SneakyThrows
    private static Supplier<StressTestOperationExecuter> memDbExecutorProvider() {
        return () -> new MemDbStressTestOperationExecuter(
                getMemDbObject(), argGenerator()
        );
    }

    @SneakyThrows
    private static MemDb getMemDbObject() {
        return new MemDb(MemDbConnections.sync("127.0.0.1", 10000), "123");
    }

    private static Supplier<String> argGenerator() {
        return () -> Character.toString((char) ((Math.random() * 90 - 65) + 65));
    }
}
