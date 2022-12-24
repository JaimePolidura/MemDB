package es.memdb.messages.request;

import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public final class RequestSerializer {
    public byte[] serialize(Request request) {
        List<Byte> auth = getAuth(request);
        List<Byte> operation = this.getOperations(request);

        byte[] totalRaw = new byte[auth.size() + operation.size()];

        for (int i = 0; i < auth.size(); i++)
            totalRaw[i] = auth.get(i);
        for (int i = 0; i < operation.size(); i++)
            totalRaw[i + auth.size()] = operation.get(i);

        return totalRaw;
    }

    private List<Byte> getOperations(Request request) {
        List<Byte> bytes = new ArrayList<>();

        bytes.add(this.getOperatorDesc(request.getOperationRequest()));

        List<String> argsString = request.getOperationRequest().getArgs();
        for (String arg : argsString) {
            int argSize = arg.length();

            bytes.add((byte) argSize);
            bytes.addAll(this.stringToBytes(arg));
        }

        return bytes;
    }

    private byte getOperatorDesc(OperationRequest operationRequest) {
        byte operatorNumber = (byte) (operationRequest.getOperator().getOperatorNumber() << 2);
        byte flag1 = (byte) (booleanToInt(operationRequest.isFlag1()) << 1);
        byte flag2 = (byte) booleanToInt(operationRequest.isFlag2());

        return (byte) (operatorNumber | flag1 | flag2);
    }

    private List<Byte> getAuth(Request request) {
        List<Byte> bytesAuth = new ArrayList<>(request.getAuthentication().getAuthKey().length() + 1);

        AuthenticationRequest authenticationNotSerialized = request.getAuthentication();
        byte authLength = (byte) (authenticationNotSerialized.getAuthKey().length() << 2);
        byte flag1 = (byte) (booleanToInt(authenticationNotSerialized.isFlag1()) << 1);
        byte flag2 = (byte) booleanToInt(request.getOperationRequest().isFlag2());

        bytesAuth.add((byte) (authLength | flag1 | flag2));
        bytesAuth.addAll(this.stringToBytes(request.getAuthentication().getAuthKey()));

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
