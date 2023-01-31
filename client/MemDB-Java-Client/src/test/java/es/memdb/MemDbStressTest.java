package es.memdb;

import es.memdb.connection.MemDbConnections;
import es.memdb.stresstest.StressTestOperation;
import es.memdb.stresstest.StressTestResult;
import es.memdb.stresstest.StressTestRunner;
import es.memdb.stresstest.executer.MemDbStressTestOperationExecuter;
import es.memdb.stresstest.executer.RedisStressTestOperationExecuter;
import es.memdb.stresstest.executer.StressTestOperationExecuter;
import lombok.SneakyThrows;
import redis.clients.jedis.Jedis;

import java.text.DecimalFormat;
import java.util.*;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicLong;
import java.util.function.Supplier;
import java.util.stream.Collectors;

//Redis docker command: docker run -d \
//    --restart unless-stopped \
//    -p 6379:6379 \
//    -e "REDIS_PASSWORD=123" \
//    redis:6.2-alpine \
//    redis-server --requirepass 123
//
// docker run -d --restart unless-stopped -p 6379:6379 -e "REDIS_PASSWORD=123" redis:6.2-alpine redis-server --requirepass 123
public final class MemDbStressTest {
    @SneakyThrows
    public static void main(String[] args) {
        int[] numberThreads = new int[]{1, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 64};
//        int[] numberThreads = new int[]{64};

        System.out.println("-------------------------------------------- THREADS TEST --------------------------------------------");
        System.out.println();
        System.out.println();

        List<StressTestOperatorAverageResult> results = new ArrayList<>();

        for (int j = 0; j < numberThreads.length; j++) {
            List<StressTestOperatorAverageResult> resultMemDb = runAndPrintAverage(10_000, numberThreads[j], memDbExecutorProvider(), "MemDb");
            List<StressTestOperatorAverageResult> resultRedis = runAndPrintAverage(10_000, numberThreads[j], redisExecutorProvider(), "Redis");

            results.addAll(resultMemDb);
            results.addAll(resultRedis);

            System.out.println();
            System.out.println();
        }


        printResultsExcelWay(results);
    }

    private static List<StressTestOperatorAverageResult> runAndPrintAverage(int numberOperations, int numberThreads, Supplier<StressTestOperationExecuter> memDbExecutorProvider,
                                           String name) {

        List<StressTestOperatorAverageResult> stressTestOperatorAverageResults = new ArrayList<>();
        StressTestRunner memDbStressTestRunner = new StressTestRunner(
                argGenerator(),
                numberOperations,
                numberThreads
        );

        System.out.println("    ---------------------- operations: "+numberOperations+" threads: "+numberThreads+" "+name+" ----------------------");

        Map<StressTestOperation, List<StressTestResult>> groupedByOperator = memDbStressTestRunner.run(memDbExecutorProvider)
                .stream()
                .collect(Collectors.groupingBy(StressTestResult::operator));
        
        for (StressTestOperation operator : groupedByOperator.keySet()) {
            double average = groupedByOperator.get(operator).stream()
                    .mapToLong(StressTestResult::time)
                    .average()
                    .getAsDouble();

            stressTestOperatorAverageResults.add(new StressTestOperatorAverageResult(
                    name, operator, average, numberOperations, numberThreads
            ));

            System.out.println("    " + operator.toString() + ": " + average);
        }

        return stressTestOperatorAverageResults;
    }

    private static void printResultsExcelWay(List<StressTestOperatorAverageResult> results) {
        DecimalFormat df = new DecimalFormat("0.00");

        Map<StressTestOperation, List<StressTestOperatorAverageResult>> groupedByOperator = results.stream()
                .collect(Collectors.groupingBy(StressTestOperatorAverageResult::operator));

        for (StressTestOperation operation : groupedByOperator.keySet()) {
            List<StressTestOperatorAverageResult> byOperation = groupedByOperator.get(operation);

            Map<String, List<StressTestOperatorAverageResult>> groupedByOperatorAndName = byOperation.stream()
                    .collect(Collectors.groupingBy(StressTestOperatorAverageResult::name));

            for (String name : groupedByOperatorAndName.keySet()) {
                List<StressTestOperatorAverageResult> byNameAndOperationSortedByThreads = groupedByOperatorAndName.get(name).stream()
                        .sorted(Comparator.comparing(StressTestOperatorAverageResult::numberThreads))
                        .toList();

                System.out.print(String.format("%s %s\t", name, operation.alias));
                StringBuilder resultsPrint = new StringBuilder();

                for (StressTestOperatorAverageResult byNameAndOperationSortedByOperation : byNameAndOperationSortedByThreads)
                    resultsPrint.append(df.format(Math.round(byNameAndOperationSortedByOperation.average * 100.0) / 100.0)).append("\t");

                System.out.print(resultsPrint);
                System.out.println();
            }
        }
    }

    private static Supplier<StressTestOperationExecuter> redisExecutorProvider() {
        return () -> new RedisStressTestOperationExecuter(
                getRedisObject()
        );
    }

    @SneakyThrows
    private static Jedis getRedisObject() {
        Jedis jedis = new Jedis("127.0.0.1", 6379);
        jedis.auth("123");
        return jedis;
    }

    private static Supplier<StressTestOperationExecuter> memDbExecutorProvider() {
        return () -> new MemDbStressTestOperationExecuter(
                getMemDbObject()
        );
    }

    @SneakyThrows
    private static MemDb getMemDbObject() {
        return new MemDb(MemDbConnections.async("127.0.0.1", 10000), "123");
    }

    private static Supplier<String> argGenerator() {
        return () -> getRandomAsciiCharacter() + getRandomAsciiCharacter();
    }

    private static String getRandomAsciiCharacter() {
        return Character.toString((char) (int) ((Math.random() * (90 - 65)) + 65));
    }

    record StressTestOperatorAverageResult(String name, StressTestOperation operator, double average, int numberOperations, int numberThreads) { }
}
