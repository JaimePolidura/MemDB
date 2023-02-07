package nodes

import (
	"encoding/json"
	"errors"
)

type NodeState uint8

const (
	RUNNING  NodeState = 0
	SHUTDOWN NodeState = 1
	SEEDING  NodeState = 2
)

func (state *NodeState) MarshalJSON() ([]byte, error) {
	valueInt := *state

	switch valueInt {
	case 0:
		return []byte("RUNNING"), nil
	case 1:
		return []byte("SHUTDOWN"), nil
	case 2:
		return []byte("SEEDING"), nil
	default:
		return nil, errors.New("not found")
	}
}

func (state *NodeState) UnmarshalJSON(data []byte) error {
	var stateValueString string
	if err := json.Unmarshal(data, &stateValueString); err != nil {
		return err
	}

	switch stateValueString {
	case "RUNNING":
		*state = RUNNING
	case "SHUTDOWN":
		*state = SHUTDOWN
	case "SEEDING":
		*state = SEEDING
	}

	return nil
}
