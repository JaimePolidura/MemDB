package main

import (
	"crypto/md5"
	"encoding/binary"
	"fmt"
)

func main() {
	value := computeMD5Hash("hola")

	fmt.Println(value)

	clusterManager := CreateClusterManager()
	clusterManager.start()
}

func computeMD5Hash(s string) uint64 {
	hash := md5.Sum([]byte(s))
	return binary.BigEndian.Uint64(hash[:])
}