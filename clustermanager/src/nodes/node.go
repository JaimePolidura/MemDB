package nodes

import "clustermanager/src/nodes/states"

type Node struct {
	NodeId  uint32           `json:"nodeId"`
	Address string           `json:"address"`
	State   states.NodeState `json:"state"`
}
