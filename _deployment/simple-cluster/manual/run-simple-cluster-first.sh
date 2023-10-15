cd ../../..
rm -r /etc/memdb1
rm -r /etc/memdb2

# CLUSTER DB
etcdctl del --prefix ""

etcdctl put /nodes/1 "{\"nodeId\":\"1\",\"address\":\"127.0.0.1:10000\",\"state\":\"BOOTING\"}"
etcdctl put /nodes/2 "{\"nodeId\":\"2\",\"address\":\"127.0.0.1:10001\",\"state\":\"BOOTING\"}"

# CLUSTER MANAGER
cd ./clustermanager

./build-and-run.sh &
sleep 2s

cd ..

# CLUSTER NODES
cd ./core
sudo ./linux-build.sh

sudo ./run.sh USE_REPLICATION=true \
        SHOW_DEBUG_LOG=true \
        SERVER_PORT=10000 \
        CLUSTER_MANAGER_ADDRESS=127.0.0.1:8080 \
        ETCD_ADDRESSES=127.0.0.1:2379 \
        DATA_PATH=/etc/memdb1 \
        NODE_ID=1