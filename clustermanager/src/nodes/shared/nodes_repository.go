package shared

type NodeRepository interface {
	FindAll() (nodes []Node, err error)
	FindById(nodeId NodeId_t) (node Node, err error)
	FindByAddress(address string) (node Node, err error)
	Add(nodeToAdd Node) error
}
