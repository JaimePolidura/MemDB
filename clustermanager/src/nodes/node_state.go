package nodes

type NodeState uint8

const (
	RUNNING  NodeState = 0
	SHUTDOWN NodeState = 1
	SEEDING  NodeState = 2
)
