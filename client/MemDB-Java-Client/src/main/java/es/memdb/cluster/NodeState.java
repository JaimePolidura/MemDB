package es.memdb.cluster;

public enum NodeState {
    RUNNING(true),
    SHUTDOWN(false),
    BOOTING(false);

    public final boolean canSendRequest;

    NodeState(boolean canSendRequest) {
        this.canSendRequest = canSendRequest;
    }
}
