package es.memdb.cluster.messages;

import lombok.AllArgsConstructor;
import lombok.Getter;

@AllArgsConstructor
public final class LoginResponse {
    @Getter private final String token;
}
