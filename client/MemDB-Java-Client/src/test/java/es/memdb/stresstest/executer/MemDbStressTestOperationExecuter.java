package es.memdb.stresstest.executer;

import es.memdb.MemDb;
import es.memdb.stresstest.StressTestOperation;
import lombok.AllArgsConstructor;

import java.util.function.Supplier;

@AllArgsConstructor
public final class MemDbStressTestOperationExecuter implements StressTestOperationExecuter {
    private final MemDb memDb;
    private final Supplier<String> argGenerator;

    @Override
    public void execute(StressTestOperation stressTestOperation) {
        switch (stressTestOperation) {
            case DELETE -> this.memDb.delete(this.argGenerator.get());
            case GET -> this.memDb.get(this.argGenerator.get());
            case SET -> this.memDb.set(argGenerator.get(), argGenerator.get());
        }
    }
}
