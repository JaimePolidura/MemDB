docker run -d -p 2379:2379 --name memdb-clusterdb  quay.io/coreos/etcd:latest

docker exec memdb-clusterdb etcdctl put /nodes/1 "{\"nodeId\": \"1\", \"address\": \"127.0.0.1:10000\", \"state\": \"RUNNING\"}"
docker exec memdb-clusterdb etcdctl put /nodes/2 "{\"nodeId\": \"2\", \"address\": \"127.0.0.1:10001\", \"state\": \"RUNNING\"}"

docker run -d -p 8080:8080 -e MEMDB_CLUSTERMANAGER_HEALTHCHECK_PERIOD=10 memdb-clustermanager:1.0

docker run -d -p 10000:10000 -e PORT=10000 -e PERSISTANCE_WRITE_EVERY=1 -e USE_REPLICATION=true memdb-core:1.0
docker run -d -p 10001:10001 -e PORT=10001 -e PERSISTANCE_WRITE_EVERY=1 -e USE_REPLICATION=true memdb-core:1.0
