package es.memdb.cluster;

import lombok.Getter;

public final class Node {
    @Getter private int nodeId;
    @Getter private NodeState state;
    @Getter private String address;
}
