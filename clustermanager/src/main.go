package main

import "os"

func main() {
	clusterManager := CreateClusterManager(os.Args)
	clusterManager.start()
}
