package es.memdb.messages.response;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Getter;

@Builder
@AllArgsConstructor
public final class Response {
    @Getter private final int errorCode;
    @Getter private final boolean isSuccessful;
    @Getter private final String response;
}
