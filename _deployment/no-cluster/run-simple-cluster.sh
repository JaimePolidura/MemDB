docker-compose down -v --remove-orphans
docker stop $(docker ps -a -q --filter "name=memdb-*")
docker rm $(docker ps -a -q --filter "name=memdb-*")

docker-compose up --no-deps memdb-instance-1