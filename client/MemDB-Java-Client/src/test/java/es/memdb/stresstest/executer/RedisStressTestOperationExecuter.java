package es.memdb.stresstest.executer;

import es.memdb.stresstest.StressTestOperation;
import lombok.AllArgsConstructor;
import redis.clients.jedis.Jedis;

import java.util.function.Supplier;

@AllArgsConstructor
public final class RedisStressTestOperationExecuter implements StressTestOperationExecuter {
    private final Jedis jedis;

    @Override
    public void execute(StressTestOperation stressTestOperation, Supplier<String> argGenerator) {
        switch (stressTestOperation) {
            case SET -> this.jedis.set(argGenerator.get(), argGenerator.get());
            case GET -> this.jedis.get(argGenerator.get());
            case DELETE -> this.jedis.del(argGenerator.get());
        }
    }
}
