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

sudo ./run.sh MEMDB_CORE_USE_REPLICATION=true \
        MEMDB_CORE_SHOW_DEBUG_LOG=true \
        MEMDB_CORE_PORT=10000 \
        MEMDB_CORE_PERSISTANCE_WRITE_EVERY=1 \
        MEMDB_CORE_CLUSTER_MANAGER_ADDRESS=127.0.0.1:8080 \
        MEMDB_CORE_ETCD_ADDRESSES=127.0.0.1:2379 \
        MEMDB_CORE_DATA_PATH=/etc/memdb1 \
        MEMDB_CORE_NODE_ID=1 &

sleep 10s

sudo ./run.sh MEMDB_CORE_USE_REPLICATION=true \
        MEMDB_CORE_SHOW_DEBUG_LOG=true \
        MEMDB_CORE_PORT=10000 \
        MEMDB_CORE_PERSISTANCE_WRITE_EVERY=1 \
        MEMDB_CORE_CLUSTER_MANAGER_ADDRESS=127.0.0.1:8080 \
        MEMDB_CORE_ETCD_ADDRESSES=127.0.0.1:2379 \
        MEMDB_CORE_DATA_PATH=/etc/memdb2 \
        MEMDB_CORE_NODE_ID=1
