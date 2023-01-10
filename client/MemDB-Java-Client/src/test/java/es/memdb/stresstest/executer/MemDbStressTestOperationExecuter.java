package es.memdb.stresstest.executer;

import es.memdb.MemDb;
import es.memdb.stresstest.StressTestOperation;
import lombok.AllArgsConstructor;

import java.util.function.Supplier;

@AllArgsConstructor
public final class MemDbStressTestOperationExecuter implements StressTestOperationExecuter {
    private final MemDb memDb;

    @Override
    public void execute(StressTestOperation stressTestOperation, Supplier<String> argGenerator) {
        switch (stressTestOperation) {
            case DELETE -> this.memDb.delete(argGenerator.get());
            case GET -> this.memDb.get(argGenerator.get());
            case SET -> this.memDb.set(argGenerator.get(), argGenerator.get());
        }
    }
}
