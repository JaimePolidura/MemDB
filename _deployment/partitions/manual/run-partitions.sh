cd ../../..
rm -r /etc/memdb1
rm -r /etc/memdb2
rm -r /etc/memdb3
rm -r /etc/memdb4

# CLUSTER DB
etcdctl del --prefix ""

etcdctl put /partitions/config/nodesPerPartition "2"
etcdctl put /partitions/config/ringSize "64"

etcdctl put /partitions/ring/1 "{\"nodeId\":\"1\",\"ringPosition\": 2}"
etcdctl put /partitions/ring/4 "{\"nodeId\":\"4\",\"ringPosition\": 29}"
etcdctl put /partitions/ring/2 "{\"nodeId\":\"2\",\"ringPosition\": 35}"
etcdctl put /partitions/ring/3 "{\"nodeId\":\"3\",\"ringPosition\": 62}"

etcdctl put /nodes/1 "{\"nodeId\":\"1\",\"address\":\"memdb-clusternode-1:10000\",\"state\":\"BOOTING\"}"
etcdctl put /nodes/2 "{\"nodeId\":\"2\",\"address\":\"memdb-clusternode-2:10001\",\"state\":\"BOOTING\"}"
etcdctl put /nodes/3 "{\"nodeId\":\"3\",\"address\":\"memdb-clusternode-3:10002\",\"state\":\"BOOTING\"}"
etcdctl put /nodes/4 "{\"nodeId\":\"4\",\"address\":\"memdb-clusternode-4:10003\",\"state\":\"BOOTING\"}"

# CLUSTER MANAGER
cd ./clustermanager

./build-and-run.sh MEMDB_CLUSTERMANAGER_HEALTHCHECK_PERIOD=240 \
       MEMDB_CLUSTERMANAGER_DO_LOGGING=true \
       MEMDB_CLUSTERMANAGER_ETCD_ENDPOINTS=127.0.0.0:2379 \ 
       MEMDB_CLUSTERMANAGER_USE_PARTITIONS=true &
	
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
        MEMDB_CORE_NODE_ID=1 \ 
	MEMDB_CORE_USE_PARTITIONS=true &

sudo ./run.sh MEMDB_CORE_USE_REPLICATION=true \
        MEMDB_CORE_SHOW_DEBUG_LOG=true \
        MEMDB_CORE_PORT=10001 \
        MEMDB_CORE_PERSISTANCE_WRITE_EVERY=1 \
        MEMDB_CORE_CLUSTER_MANAGER_ADDRESS=127.0.0.1:8080 \
        MEMDB_CORE_ETCD_ADDRESSES=127.0.0.1:2379 \
        MEMDB_CORE_DATA_PATH=/etc/memdb2 \
        MEMDB_CORE_NODE_ID=2 \ 
        MEMDB_CORE_USE_PARTITIONS=true &

sudo ./run.sh MEMDB_CORE_USE_REPLICATION=true \
        MEMDB_CORE_SHOW_DEBUG_LOG=true \
        MEMDB_CORE_PORT=10002 \
        MEMDB_CORE_PERSISTANCE_WRITE_EVERY=1 \
        MEMDB_CORE_CLUSTER_MANAGER_ADDRESS=127.0.0.1:8080 \
        MEMDB_CORE_ETCD_ADDRESSES=128.0.0.1:2379 \
        MEMDB_CORE_DATA_PATH=/etc/memdb3 \
        MEMDB_CORE_NODE_ID=3 \
        MEMDB_CORE_USE_PARTITIONS=true &

sudo ./run.sh MEMDB_CORE_USE_REPLICATION=true \
        MEMDB_CORE_SHOW_DEBUG_LOG=true \
        MEMDB_CORE_PORT=10003 \
        MEMDB_CORE_PERSISTANCE_WRITE_EVERY=1 \
        MEMDB_CORE_CLUSTER_MANAGER_ADDRESS=127.0.0.1:8080 \
        MEMDB_CORE_ETCD_ADDRESSES=127.0.0.1:2379 \
        MEMDB_CORE_DATA_PATH=/etc/memdb4 \
        MEMDB_CORE_NODE_ID=4 \
        MEMDB_CORE_USE_PARTITIONS=true &

sudo ./run.sh MEMDB_CORE_USE_REPLICATION=true \
        MEMDB_CORE_SHOW_DEBUG_LOG=true \
        MEMDB_CORE_PORT=10004 \
        MEMDB_CORE_PERSISTANCE_WRITE_EVERY=1 \
        MEMDB_CORE_CLUSTER_MANAGER_ADDRESS=127.0.0.1:8080 \
        MEMDB_CORE_ETCD_ADDRESSES=127.0.0.1:2379 \
        MEMDB_CORE_DATA_PATH=/etc/memdb5 \
        MEMDB_CORE_NODE_ID=5 \
        MEMDB_CORE_USE_PARTITIONS=true &

sudo ./run.sh MEMDB_CORE_USE_REPLICATION=true \
        MEMDB_CORE_SHOW_DEBUG_LOG=true \
        MEMDB_CORE_PORT=10005 \
        MEMDB_CORE_PERSISTANCE_WRITE_EVERY=1 \
        MEMDB_CORE_CLUSTER_MANAGER_ADDRESS=127.0.0.1:8080 \
        MEMDB_CORE_ETCD_ADDRESSES=127.0.0.1:2379 \
        MEMDB_CORE_DATA_PATH=/etc/memdb6 \
        MEMDB_CORE_NODE_ID=6 \
        MEMDB_CORE_USE_PARTITIONS=true

