package es.memdb.cluster;

import java.util.List;

public record ClusterConfig(int nodesPerPartition,
                            int maxPartitionSize,
                            List<ClusterConfigNode> clusterNodes,
                            List<ClusterConfigRingEntries> ringEntries) {

    public boolean usingPartitions() {
        return nodesPerPartition == 0 && maxPartitionSize == 0;
    }

    public int getPartitionId(String key) {
        if(usingPartitions()){

        } else {
            return 0;
        }
    }
}

record ClusterConfigNode(int nodeId, String address) {}

record ClusterConfigRingEntries(int nodeId, int ringPosition) {}
