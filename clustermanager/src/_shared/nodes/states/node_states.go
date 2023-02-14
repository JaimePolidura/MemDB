package states

type NodeState string

const (
	RUNNING  NodeState = "RUNNING"
	SHUTDOWN NodeState = "SHUTDOWN"
	SEEDING  NodeState = "SEEDING"
)
