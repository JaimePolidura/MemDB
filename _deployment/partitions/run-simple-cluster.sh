docker images prune
docker-compose down -v --remove-orphans
docker stop $(docker ps -a -q --filter "name=memdb-*")
docker rm $(docker ps -a -q --filter "name=memdb-*")

docker-compose up -d memdb-clusterdb &
sleep 5s

docker-compose exec memdb-clusterdb etcdctl put /partitions/config/nodesPerPartition "2"
docker-compose exec memdb-clusterdb etcdctl put /partitions/config/ringSize "64"

docker-compose exec memdb-clusterdb etcdctl put /partitions/ring/2  "{\"nodeId\":\"1\",\"ringPosition\":2}"
docker-compose exec memdb-clusterdb etcdctl put /partitions/ring/29 "{\"nodeId\":\"4\",\"ringPosition\":29}"
docker-compose exec memdb-clusterdb etcdctl put /partitions/ring/35 "{\"nodeId\":\"2\",\"ringPosition\":35}"
docker-compose exec memdb-clusterdb etcdctl put /partitions/ring/62 "{\"nodeId\":\"3\",\"ringPosition\":62}"

docker-compose exec memdb-clusterdb etcdctl put /nodes/1 "{\"nodeId\":\"1\",\"address\":\"memdb-clusternode-1:10000\",\"state\":\"BOOTING\"}"
docker-compose exec memdb-clusterdb etcdctl put /nodes/2 "{\"nodeId\":\"2\",\"address\":\"memdb-clusternode-2:10001\",\"state\":\"BOOTING\"}"
docker-compose exec memdb-clusterdb etcdctl put /nodes/3 "{\"nodeId\":\"3\",\"address\":\"memdb-clusternode-3:10002\",\"state\":\"BOOTING\"}"
docker-compose exec memdb-clusterdb etcdctl put /nodes/4 "{\"nodeId\":\"4\",\"address\":\"memdb-clusternode-4:10003\",\"state\":\"BOOTING\"}"

docker-compose up --no-deps memdb-clustermanager memdb-clusternode-1 memdb-clusternode-2 memdb-clusternode-3 memdb-clusternode-4