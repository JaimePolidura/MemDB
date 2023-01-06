package es.memdb;

import es.memdb.connection.MemDbConnections;
import lombok.Getter;
import lombok.SneakyThrows;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Random;
import java.util.function.Supplier;

import static es.memdb.Operator.*;

public final class MemDbStressTest {
    private static Random random = new Random();
    private static boolean LOGGING = false;

    @SneakyThrows
    public static void main(String[] args) {
        System.out.println("---------------------- START 1 THREADS: ----------------------");
        List<StressTestActionResult> result = runTest(1, 1000000);
    }

    @SneakyThrows
    private static List<StressTestActionResult> runTest(int numberThreads, int numberOperations) {
        MemDb memDb = new MemDb(MemDbConnections.async("127.0.0.1", 10000), "123");

        StressTestThread[] threads = new StressTestThread[numberThreads];
        for (int i = 0; i < numberThreads; i++)
            threads[i] = new StressTestThread(memDb, numberOperations, randomLetter());
        for (int i = 0; i < numberThreads; i++)
            threads[i].start();
        for (int i = 0; i < numberThreads; i++)
            threads[i].join();

        List<StressTestActionResult> results = new ArrayList<>(numberOperations * numberThreads);
        for (StressTestThread thread : threads)
            results.addAll(thread.getResults());

        return results;
    }

    private static Supplier<String> randomLetter() {
        return () -> Character.toString((char) ((Math.random() * 90 - 65) + 65));
    }

    private static class StressTestThread extends Thread {
        @Getter private final List<StressTestActionResult> results;
        private final Supplier<String> argGenerator;
        private final int numberOperations;
        private final MemDb memDb;

        public StressTestThread(MemDb memDb, int numberOperations, Supplier<String> argGenerator) {
            this.numberOperations = numberOperations;
            this.argGenerator = argGenerator;
            this.results = new ArrayList<>(numberOperations);
            this.memDb = memDb;
        }

        @Override
        public void run() {
            for(int i = 0; i < this.numberOperations; i++){
                int operatorNumber = random.nextInt(0, 2) + 1;

                Operator operatorPerform = Arrays.stream(values())
                        .filter(operator -> operator.operatorNumber == operatorNumber)
                        .findFirst()
                        .get();

                long result = this.execute(operatorPerform);
                this.results.add(new StressTestActionResult(result, operatorPerform));
            }
        }

        private long execute(Operator operatorPerform) {
           return switch (operatorPerform) {
                case SET -> this.sendRequest(SET, (() -> this.memDb.set(argGenerator.get(), argGenerator.get())));
                case GET -> this.sendRequest(GET, () -> this.memDb.get(argGenerator.get()));
                case DELETE -> this.sendRequest(DELETE, () -> this.memDb.delete(argGenerator.get()));
            };
        }

        private long sendRequest(Operator operator, Runnable request) {
            long a = System.currentTimeMillis();
            request.run();
            long b = System.currentTimeMillis();

            if(LOGGING) System.out.println("Executed " + operator.toString() + " in " + (b - a) + "ms");

            return b - a;
        }
    }

    private record StressTestActionResult(long time, Operator operator){}
}
