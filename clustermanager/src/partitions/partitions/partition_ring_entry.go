package partitions

import "clustermanager/src/nodes/nodes"

type PartitionRingEntry struct {
	NodeId       nodes.NodeId_t `json:"nodeId"`
	RingPosition uint32         `json:"ringPosition"`
}
