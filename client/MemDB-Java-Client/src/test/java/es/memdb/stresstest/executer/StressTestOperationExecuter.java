package es.memdb.stresstest.executer;

import es.memdb.stresstest.StressTestOperation;

import java.util.function.Supplier;

public interface StressTestOperationExecuter {
    void execute(StressTestOperation stressTestOperation, Supplier<String> argGenerator);
}
