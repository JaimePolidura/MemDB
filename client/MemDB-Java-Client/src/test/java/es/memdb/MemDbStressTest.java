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

import java.util.List;
import java.util.Map;
import java.util.concurrent.TimeUnit;
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

        for (int i = 0; i < 3; i++) {
            System.out.println("-------------------------------------------- ROUND "+i+" --------------------------------------------");
            System.out.println();
            System.out.println();

            System.out.println("    -------------------------------------------- THREADS TEST --------------------------------------------");
            System.out.println();
            System.out.println();

            for (int j = 0; j < numberThreads.length; j++) {
                runAndPrintAverage(10_000, numberThreads[j], memDbExecutorProvider(), "MemDb");
                runAndPrintAverage(10_000, numberThreads[j], redisExecutorProvider(), "Redis");

                System.out.println();
                System.out.println();
            }
        }
    }

    private static void runAndPrintAverage(int numberOperations, int numberThreads, Supplier<StressTestOperationExecuter> memDbExecutorProvider,
                                           String name) {
        StressTestRunner memDbStressTestRunner = new StressTestRunner(
                argGenerator(),
                numberOperations,
                numberThreads
        );

        System.out.println("        ---------------------- operations: "+numberOperations+" threads: "+numberThreads+" "+name+" ----------------------");

        long a = System.currentTimeMillis();
        List<StressTestResult> results = memDbStressTestRunner.run(memDbExecutorProvider);
        long b = System.currentTimeMillis();

        Map<StressTestOperation, List<StressTestResult>> groupedByOperator = results.stream()
                .collect(Collectors.groupingBy(StressTestResult::operator));

        System.out.println("        Finished in total time: " + TimeUnit.MILLISECONDS.toSeconds(b - a) + "s");
        for (StressTestOperation operator : groupedByOperator.keySet()) {
            double average = groupedByOperator.get(operator).stream()
                    .mapToLong(StressTestResult::time)
                    .average()
                    .getAsDouble();

            System.out.println("        " + operator.toString() + ": " + average);
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
}
