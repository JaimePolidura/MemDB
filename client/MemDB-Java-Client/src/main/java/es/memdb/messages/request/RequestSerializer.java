package es.memdb.messages.request;

import es.memdb.Utils;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public final class RequestSerializer {
    public byte[] serialize(Request request, long timestamp) {
        List<Byte> requestNumber = this.getRequestNumber(request);
        List<Byte> auth = this.getAuth(request);
        List<Byte> operation = this.getOperations(request, timestamp);
        int totalSize = requestNumber.size() + auth.size() + operation.size();

        byte[] serialized = new byte[4 + requestNumber.size() + auth.size() + operation.size()];

        serialized = ByteBuffer.wrap(serialized)
                .order(ByteOrder.BIG_ENDIAN)
                .putInt(totalSize)
                .array();
        for (int i = 0; i < requestNumber.size(); i++)
            serialized[i + 4] = requestNumber.get(i);
        for (int i = 0; i < auth.size(); i++)
            serialized[i + 4 + requestNumber.size()] = auth.get(i);
        for (int i = 0; i < operation.size(); i++)
            serialized[i + 4 + requestNumber.size() + auth.size()] = operation.get(i);

        return serialized;
    }

    private List<Byte> getRequestNumber(Request request) {
        byte[] bytes = ByteBuffer.allocate(Integer.BYTES)
                .order(ByteOrder.BIG_ENDIAN)
                .putInt(request.getRequestNumber())
                .array();

        return Arrays.stream(Utils.primitiveToWrapper(bytes)).toList();
    }

    private List<Byte> getOperations(Request request, long timestamp) {
        List<Byte> bytes = new ArrayList<>();

        bytes.add(this.getOperatorDesc(request.getOperationRequest()));
        bytes.addAll(this.getTimestamp(timestamp));

        List<String> argsString = request.getOperationRequest().getArgs();
        for (String arg : argsString) {
            int argSize = arg.length();

            var sizeArg = Utils.primitiveToWrapper(ByteBuffer.allocate(4).putInt(argSize).array());
            bytes.addAll(Arrays.asList(sizeArg));
            bytes.addAll(this.stringToBytes(arg));
        }

        return bytes;
    }

    private byte getOperatorDesc(OperationRequest operationRequest) {
        byte operatorNumber = (byte) (operationRequest.getOperator().operatorNumber << 2);
        byte flag1 = (byte) (booleanToInt(operationRequest.isFlag1()) << 1);
        byte flag2 = (byte) booleanToInt(operationRequest.isFlag2());

        return (byte) (operatorNumber | flag1 | flag2);
    }

    private List<Byte> getTimestamp(long timestamp) {
        byte[] bytes = ByteBuffer.allocate(Long.BYTES)
                .order(ByteOrder.BIG_ENDIAN)
                .putLong(timestamp)
                .array();

        return Arrays.stream(Utils.primitiveToWrapper(bytes)).toList();
    }

    private List<Byte> getAuth(Request request) {
        List<Byte> bytesAuth = new ArrayList<>(request.getAuthentication().getAuthApiKey().length() + 1);

        AuthenticationRequest authenticationNotSerialized = request.getAuthentication();
        byte authLength = (byte) (authenticationNotSerialized.getAuthApiKey().length() << 2);
        byte flag1 = (byte) (booleanToInt(authenticationNotSerialized.isFlag1()) << 1);
        byte flag2 = (byte) booleanToInt(request.getOperationRequest().isFlag2());

        bytesAuth.add((byte) (authLength | flag1 | flag2));
        bytesAuth.addAll(this.stringToBytes(request.getAuthentication().getAuthApiKey()));

        return bytesAuth;
    }


    private List<Byte> stringToBytes(String string) {
        byte[] bytesRaw = string.getBytes(StandardCharsets.US_ASCII);
        List<Byte> bytes = new ArrayList<>(bytesRaw.length);

        for (byte b : bytesRaw) bytes.add(b);

        return bytes;
    }

    private int booleanToInt(boolean value) {
        return value ? 1 : 0;
    }
}
