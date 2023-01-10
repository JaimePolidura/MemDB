package es.memdb.stresstest.executer;

import es.memdb.stresstest.StressTestOperation;

public interface StressTestOperationExecuter {
    void execute(StressTestOperation stressTestOperation);
}
