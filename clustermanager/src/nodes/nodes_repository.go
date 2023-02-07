package nodes

type NodeRepository interface {
	findAll() []Node
	findById(nodeId uint32) (node Node, err error)
	add(nodeToAdd Node) error
}
