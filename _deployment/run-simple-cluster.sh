docker compose rm -f
docker stop $(docker ps -a -q --filter "name=memdb-*")
docker rm $(docker ps -a -q --filter "name=memdb-*")

docker compose up --force-recreate

#docker exec memdb-clusterdb etcdctl put /nodes/1 "{\"nodeId\": \"1\", \"address\": \"memdb-clusternode-1:10000\", \"state\": \"RUNNING\"}"
#docker exec memdb-clusterdb etcdctl put /nodes/2 "{\"nodeId\": \"2\", \"address\": \"memdb-clusternode-2:10001\", \"state\": \"RUNNING\"}"