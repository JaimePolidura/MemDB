package es.memdb.cluster.messages;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;

@AllArgsConstructor
@NoArgsConstructor
public final class LoginResponse {
    @Getter private String token;
}
