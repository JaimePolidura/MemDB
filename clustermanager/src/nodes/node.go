package nodes

import (
	"clustermanager/src/nodes/states"
)

type NodeId_t string

type Node struct {
	NodeId  NodeId_t         `json:"nodeId"`
	Address string           `json:"address"`
	State   states.NodeState `json:"state"`
}

func (node *Node) WithState(state states.NodeState) *Node {
	node.State = state
	return node
}

func (node *Node) IsInErrorState() bool {
	return node.State == states.SHUTDOWN
}

func (node *Node) WithRunningState() *Node {
	node.State = states.RUNNING
	return node
}

func (node *Node) WithErrorState() *Node {
	node.State = states.SHUTDOWN
	return node
}
