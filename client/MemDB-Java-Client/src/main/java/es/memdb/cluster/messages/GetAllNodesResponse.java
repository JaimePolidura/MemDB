package es.memdb.cluster.messages;

import es.memdb.cluster.Node;
import lombok.AllArgsConstructor;
import lombok.Getter;

import java.util.List;

@AllArgsConstructor
public final class GetAllNodesResponse {
    @Getter private final List<Node> nodes;
}
