package utils

import (
	"encoding/binary"
	"hash"
)

type HashCalculator struct {
	HashAlgorithm hash.Hash
}

func (hashCalculator HashCalculator) Calculate(value string) uint64 {
	hashCalculator.HashAlgorithm.Write([]byte(value))
	hash := hashCalculator.HashAlgorithm.Sum(nil)

	return binary.BigEndian.Uint64(hash[:8])
}
