package states

type NodeState string

const (
	RUNNING  NodeState = "RUNNING"
	CRITICAL NodeState = "CRITICAL"
	SHUTDOWN NodeState = "SHUTDOWN"
	SEEDING  NodeState = "SEEDING"
)

//func (state *NodeState) MarshalJSON() ([]byte, error) {
//	valueInt := *state
//
//	switch valueInt {
//	case 0:
//		return []byte("RUNNING"), nil
//	case 1:
//		return []byte("SHUTDOWN"), nil
//	case 2:
//		return []byte("SEEDING"), nil
//	default:
//		return nil, errors.New("not found")
//	}
//}
//
//func (state *NodeState) UnmarshalJSON(data []byte) error {
//	var stateValueString string
//	if err := json.Unmarshal(data, &stateValueString); err != nil {
//		return err
//	}
//
//	switch stateValueString {
//	case "RUNNING":
//		*state = RUNNING
//	case "SHUTDOWN":
//		*state = SHUTDOWN
//	case "SEEDING":
//		*state = SEEDING
//	}
//
//	return nil
//}
