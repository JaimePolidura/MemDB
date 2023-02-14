package nodes

type NodeRepository interface {
	FindAll() (nodes []Node, err error)
	FindById(nodeId uint32) (node Node, err error)
	FindByAddress(address string) (node Node, err error)
	Add(nodeToAdd Node) error
}
