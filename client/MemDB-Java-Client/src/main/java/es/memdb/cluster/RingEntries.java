package es.memdb.cluster;

import es.memdb.Utils;
import lombok.AllArgsConstructor;

import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;

@AllArgsConstructor
public final class RingEntries {
    private RingEntryNode head;
    private int nodesPerPartition;
    private int maxPartitionSize;

    public List<Integer> getNodesIdByKey(String key) {
        long ringPosition = Utils.md5(key) % maxPartitionSize;
        var partitionHead = getPartitionHeadByKey(ringPosition);

        List<Integer> nodesId = new ArrayList<>(nodesPerPartition);
        for (int i = 0; i < maxPartitionSize; i++) {
            nodesId.add(partitionHead.nodeId);
            partitionHead = partitionHead.next;
        }

        return nodesId;
    }

    private RingEntryNode getPartitionHeadByKey(long ringPosition) {
        var actual = head;
        var next = actual.next;

        while(true) {
            if(next == head){
                return actual;
            }
            if(actual.ringPosition <= ringPosition && next.ringPosition >= ringPosition){
                return actual;
            }
            actual = next;
            next = actual.next;
        }
    }

    public static RingEntries fromClusterConfig(ClusterConfig config) {
        List<ClusterConfigRingEntry> clusterConfigRingEntries = config.ringEntries();
        clusterConfigRingEntries.sort(Comparator.comparingInt(ClusterConfigRingEntry::ringPosition));

        RingEntryNode head = null;
        RingEntryNode prev = null;

        for (int i = 0; i < clusterConfigRingEntries.size(); i++) {
            RingEntryNode newRingEntryNode = RingEntryNode.fromRingEntryClusterConfig(clusterConfigRingEntries.get(i));

            if(i == 0) {
                head = newRingEntryNode;
            } else if (i == clusterConfigRingEntries.size() - 1) { //Last iteration
                newRingEntryNode.back = prev;
                prev.next = newRingEntryNode;
                head.back = newRingEntryNode;
                newRingEntryNode.next = head;
            } else {
                newRingEntryNode.back = prev;
                prev.next = newRingEntryNode;
            }

            prev = newRingEntryNode;
        }

        return new RingEntries(head, config.nodesPerPartition(), config.maxPartitionSize());
    }

    @AllArgsConstructor
    private static class RingEntryNode {
        public RingEntryNode next;
        public RingEntryNode back;
        public int ringPosition;
        public int nodeId;

        public static RingEntryNode fromRingEntryClusterConfig(ClusterConfigRingEntry fromConfig) {
            return new RingEntryNode(null, null, fromConfig.ringPosition(), fromConfig.nodeId());
        }
    }
}
