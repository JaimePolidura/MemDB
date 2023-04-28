docker-compose down -v --remove-orphans
docker stop $(docker ps -a -q --filter "name=memdb-*")
docker rm $(docker ps -a -q --filter "name=memdb-*")

docker-compose up -d memdb-clusterdb &
sleep 5s

docker-compose exec memdb-clusterdb etcdctl put /nodes/1 "{\"nodeId\":\"1\",\"address\":\"memdb-clusternode-1:10000\",\"state\":\"BOOTING\"}"
docker-compose exec memdb-clusterdb etcdctl put /nodes/2 "{\"nodeId\":\"2\",\"address\":\"memdb-clusternode-2:10001\",\"state\":\"BOOTING\"}"

docker-compose up --no-deps memdb-clustermanager memdb-clusternode-1 memdb-clusternode-2