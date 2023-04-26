package nodes

type NodeId_t string

type Node struct {
	NodeId  NodeId_t  `json:"nodeId"`
	Address string    `json:"address"`
	State   NodeState `json:"state"`
}

func (node *Node) WithState(state NodeState) *Node {
	node.State = state
	return node
}
