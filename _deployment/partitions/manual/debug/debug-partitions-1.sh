cd ../../../..

# CLUSTER DB
etcdctl del --prefix ""

etcdctl put /partitions/config/nodesPerPartition "2"
etcdctl put /partitions/config/ringSize "64"

etcdctl put /partitions/ring/1 "{\"nodeId\":\"1\",\"ringPosition\": 2}"
etcdctl put /partitions/ring/4 "{\"nodeId\":\"4\",\"ringPosition\": 29}"
etcdctl put /partitions/ring/2 "{\"nodeId\":\"2\",\"ringPosition\": 35}"
etcdctl put /partitions/ring/3 "{\"nodeId\":\"3\",\"ringPosition\": 62}"

etcdctl put /nodes/1 "{\"nodeId\":\"1\",\"address\":\"127.0.0.1:10000\",\"state\":\"BOOTING\"}"
etcdctl put /nodes/2 "{\"nodeId\":\"2\",\"address\":\"127.0.0.1:10001\",\"state\":\"BOOTING\"}"
etcdctl put /nodes/3 "{\"nodeId\":\"3\",\"address\":\"127.0.0.1:10002\",\"state\":\"BOOTING\"}"
etcdctl put /nodes/4 "{\"nodeId\":\"4\",\"address\":\"127.0.0.1:10003\",\"state\":\"BOOTING\"}"

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

sudo gdb --args ./src/build/src/memdb_run USE_REPLICATION=true \
        SHOW_DEBUG_LOG=true \
        SERVER_PORT=10000 \
        CLUSTER_MANAGER_ADDRESS=127.0.0.1:8080 \
        ETCD_ADDRESSES=127.0.0.1:2379 \
        DATA_PATH=/etc/memdb1 \
        NODE_ID=1 \
		USE_PARTITIONS=true