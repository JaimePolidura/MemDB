package es.memdb.cluster.messages;

import es.memdb.cluster.Node;
import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;

import java.util.List;

@AllArgsConstructor
@NoArgsConstructor
public final class GetAllNodesResponse {
    @Getter private List<Node> nodes;
}
