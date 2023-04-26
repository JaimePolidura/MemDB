package nodes

type NodeState string

const (
	RUNNING  NodeState = "RUNNING"
	SHUTDOWN NodeState = "SHUTDOWN"
	BOOTING  NodeState = "BOOTING"
)
