package es.memdb.stresstest;

import es.memdb.stresstest.executer.StressTestOperationExecuter;
import lombok.*;

import java.util.ArrayList;
import java.util.List;
import java.util.function.Supplier;

@AllArgsConstructor
@Builder
public final class StressTestRunner {
    @Getter private Supplier<String> argGenerator;
    @Setter @Getter private int numberOperations;
    @Setter @Getter private int numberThreads;

    @SneakyThrows
    public List<StressTestResult> run(Supplier<StressTestOperationExecuter> operationExecuter) {
        StressTestThread[] threads = new StressTestThread[this.numberThreads];
        for (int i = 0; i < threads.length; i++)
            threads[i] = new StressTestThread(this.numberOperations, this.argGenerator, operationExecuter.get());
        for (int i = 0; i < threads.length; i++)
            threads[i].start();
        for (int i = 0; i < threads.length; i++)
            threads[i].join();

        List<StressTestResult> results = new ArrayList<>(numberOperations * numberThreads);
        for (StressTestThread thread : threads)
            results.addAll(thread.getResults());

        return results;
    }
}
