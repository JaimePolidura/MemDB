package nodes

import "clustermanager/src/nodes/states"

type Node struct {
	NodeId  uint32           `json:"nodeId"`
	Address string           `json:"address"`
	State   states.NodeState `json:"state"`
}

func (node *Node) WithState(state states.NodeState) *Node {
	node.State = state
	return node
}

func (node *Node) WithNextErrorState() *Node {
	if node.State == states.CRITICAL {
		node.State = states.RUNNING
	} else {
		node.State = states.CRITICAL
	}

	return node
}
