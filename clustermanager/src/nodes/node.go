package nodes

type Node struct {
	NodeId  uint32    `json:"nodeId"`
	Address string    `json:"address"`
	State   NodeState `json:"state"`
}
