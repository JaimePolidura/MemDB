package nodes

type Node struct {
	nodeId uint32
	ip     uint32
	port   uint16
	state  NodeState
}
