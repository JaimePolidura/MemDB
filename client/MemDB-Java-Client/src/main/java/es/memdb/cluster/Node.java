package es.memdb.cluster;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Getter;
import lombok.NoArgsConstructor;

import java.util.Objects;

@NoArgsConstructor
@AllArgsConstructor
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

    public int getPort() {
        return Integer.parseInt(this.address.split(":")[1]);
    }

    public String getIP() {
        return this.address.split(":")[0];
    }
}
