package es.memdb.messages.response;

import es.memdb.utils.LamportClock;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Getter;
import lombok.With;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.StandardCharsets;

@Builder
@AllArgsConstructor
public final class Response {
    @Getter private final long requestNumber;
    @Getter private final LamportClock timestamp;
    @Getter private final int errorCode;
    @Getter private final boolean isSuccessful;
    @Getter @With private final byte[] response;

    public boolean hasError(int errorCode) {
        return (this.errorCode & errorCode) == errorCode;
    }

    public long toLong() {
        return ByteBuffer.wrap(this.response).order(ByteOrder.BIG_ENDIAN).getLong();
    }

    public int toInt() {
        return ByteBuffer.wrap(this.response).order(ByteOrder.BIG_ENDIAN).getInt();
    }

    public String toString() {
        return new String(this.response, StandardCharsets.US_ASCII);
    }

    public boolean isFailed() {
        return !this.isSuccessful;
    }
}
