package es.memdb.cluster;

import lombok.Getter;

import java.util.Objects;

public final class Node {
    @Getter private int nodeId;
    @Getter private NodeState state;
    @Getter private String address;

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        Node node = (Node) o;
        return nodeId == node.nodeId && state == node.state && Objects.equals(address, node.address);
    }

    @Override
    public int hashCode() {
        return Objects.hash(nodeId, state, address);
    }
}
