package es.memdb;

import io.vavr.CheckedRunnable;
import lombok.SneakyThrows;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.security.MessageDigest;
import java.util.*;

public final class Utils {
    public static int toInt(byte[] bytes) {
        return ByteBuffer.wrap(bytes).getInt();
    }

    public static byte[] wrapperToPrimitive(Byte[] wrapper) {
        byte[] primitive = new byte[wrapper.length];

        for (int i = 0; i < wrapper.length; i++)
            primitive[i] = wrapper[i];

        return primitive;
    }

    public static <T> List<T> getRandomUnique(List<T> list, int n) {
        if(n >= list.size()){
            return list;
        }

        List<T> alreadyTaken = new ArrayList<>();
        while (alreadyTaken.size() < n) {
            T randomValue = list.get((int) (Math.random() * list.size()));
            if(!alreadyTaken.contains(randomValue)){
                alreadyTaken.add(randomValue);
            }
        }

        return alreadyTaken;
    }

    @SneakyThrows
    public static long md5(String key) {
        MessageDigest md = MessageDigest.getInstance("MD5");
        byte[] hashBytes = md.digest(key.getBytes());

        return ByteBuffer.wrap(hashBytes)
                .order(ByteOrder.BIG_ENDIAN)
                .getLong();
    }

    public static void rethrowNoChecked(CheckedRunnable checkedRunnable) {
        try{
            checkedRunnable.run();
        } catch (Throwable e) {
            throw new RuntimeException(e);
        }
    }

    public static Byte[] primitiveToWrapper(byte[] primitive) {
        Byte[] bytes = new Byte[primitive.length];

        for (int i = 0; i < primitive.length; i++)
            bytes[i] = primitive[i];

        return bytes;
    }
}
