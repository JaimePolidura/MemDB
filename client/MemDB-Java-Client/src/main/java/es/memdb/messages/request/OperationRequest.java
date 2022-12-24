package es.memdb.messages.request;

import es.memdb.Operator;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Getter;

import java.util.List;

@Builder
@AllArgsConstructor
public final class OperationRequest {
    @Getter private final Operator operator;
    @Getter private final boolean flag1;
    @Getter private final boolean flag2;
    @Getter private final List<String> args;
}
