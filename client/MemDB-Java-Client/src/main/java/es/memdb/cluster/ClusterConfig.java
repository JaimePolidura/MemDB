package es.memdb.cluster;

import java.util.List;

public record ClusterConfig(int nodesPerPartition,
                            int maxPartitionSize,
                            List<ClusterConfigNode> clusterNodes,
                            List<ClusterConfigRingEntry> ringEntries) {

    public boolean usingPartitions() {
        return nodesPerPartition == 0 && maxPartitionSize == 0;
    }
}

record ClusterConfigNode(int nodeId, String address) {}

record ClusterConfigRingEntry(int nodeId, int ringPosition) {}
