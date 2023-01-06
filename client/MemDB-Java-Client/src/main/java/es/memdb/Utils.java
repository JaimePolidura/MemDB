package es.memdb;

import java.nio.ByteBuffer;

public final class Utils {
    public static long toLong(byte[] bytes) {
        return ByteBuffer.wrap(bytes).getLong();
    }

    public static byte[] wrapperToPrimitive(Byte[] wrapper) {
        byte[] primitive = new byte[wrapper.length];

        for (int i = 0; i < wrapper.length; i++)
            primitive[i] = wrapper[i];

        return primitive;
    }

    public static Byte[] primitiveToWrapper(byte[] primitive) {
        Byte[] bytes = new Byte[primitive.length];

        for (int i = 0; i < primitive.length; i++)
            bytes[i] = primitive[i];

        return bytes;
    }
}
