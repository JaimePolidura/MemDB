package es.memdb.messages.request;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Getter;

@Builder
@AllArgsConstructor
public final class AuthenticationRequest {
    @Getter private final String authClientKey;
    @Getter private final boolean flag1;
    @Getter private final boolean flag2;
}
