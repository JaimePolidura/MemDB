package es.memdb;

public final class Utils {
    public static long toLong(byte[] bytes) {
        return ((long) bytes[0] << 56) | ((long) bytes[1] << 48) | ((long) bytes[2] << 40) | ((long) bytes[3] << 32) |
                ((long) bytes[4] << 24) | ((long) bytes[5] << 16) | ((long) bytes[6] << 8) | (long) bytes[7];
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
