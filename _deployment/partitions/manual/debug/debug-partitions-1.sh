cd ../../../..

# CLUSTER DB
docker images prune
docker stop $(docker ps -a -q --filter "name=memdb-*")
docker rm $(docker ps -a -q --filter "name=memdb-*")

docker run -d \
  --name memdb-clusterdb \
  -p 2379:2379 \
  -e ALLOW_NONE_AUTHENTICATION=yes \
  -e ETCDCTL_API=3 \
  --log-driver none \
  --restart always \
  bitnami/etcd:latest

sleep 3s

docker exec -it memdb-clusterdb etcdctl put /partitions/config/nodesPerPartition "2"
docker exec -it memdb-clusterdb etcdctl put /partitions/config/ringSize "64"

docker exec -it memdb-clusterdb etcdctl put /partitions/ring/1 "{\"nodeId\":\"1\",\"ringPosition\": 2}"
docker exec -it memdb-clusterdb etcdctl put /partitions/ring/4 "{\"nodeId\":\"4\",\"ringPosition\": 29}"
docker exec -it memdb-clusterdb etcdctl put /partitions/ring/2 "{\"nodeId\":\"2\",\"ringPosition\": 35}"
docker exec -it memdb-clusterdb etcdctl put /partitions/ring/3 "{\"nodeId\":\"3\",\"ringPosition\": 62}"

docker exec -it memdb-clusterdb etcdctl put /nodes/1 "{\"nodeId\":\"1\",\"address\":\"127.0.0.1:10000\",\"state\":\"BOOTING\"}"
docker exec -it memdb-clusterdb etcdctl put /nodes/2 "{\"nodeId\":\"2\",\"address\":\"127.0.0.1:10001\",\"state\":\"BOOTING\"}"
docker exec -it memdb-clusterdb etcdctl put /nodes/3 "{\"nodeId\":\"3\",\"address\":\"127.0.0.1:10002\",\"state\":\"BOOTING\"}"
docker exec -it memdb-clusterdb etcdctl put /nodes/4 "{\"nodeId\":\"4\",\"address\":\"127.0.0.1:10003\",\"state\":\"BOOTING\"}"

# CLUSTER NODES
cd ./core
sudo ./linux-build.sh

sudo gdb --args ./src/build/src/memdb_run USE_REPLICATION=true \
        SHOW_DEBUG_LOG=true \
        SERVER_PORT=10000 \
        CLUSTER_MANAGER_ADDRESS=127.0.0.1:8080 \
        ETCD_ADDRESSES=127.0.0.1:2379 \
        DATA_PATH=/etc/memdb1 \
        NODE_ID=1 \
		USE_PARTITIONS=true
