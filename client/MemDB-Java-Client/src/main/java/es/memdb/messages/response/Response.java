package es.memdb.messages.response;

import es.memdb.utils.LamportClock;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Getter;

@Builder
@AllArgsConstructor
public final class Response {
    @Getter private final long requestNumber;
    @Getter private final LamportClock timestamp;
    @Getter private final int errorCode;
    @Getter private final boolean isSuccessful;
    @Getter private final String response;

    public boolean isFailed() {
        return !this.isSuccessful;
    }
}
