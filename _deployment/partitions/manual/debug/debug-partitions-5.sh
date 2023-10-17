cd ../../../../core

etcdctl put /partitions/ring/5 "{\"nodeId\":\"5\",\"ringPosition\": 5}"
etcdctl put /nodes/5 "{\"nodeId\":\"5\",\"address\":\"127.0.0.1:10004\",\"state\":\"BOOTING\"}"

sudo ./linux-build.sh

sudo ./src/build/src/memdb_run USE_REPLICATION=true \
        SHOW_DEBUG_LOG=true \
        SERVER_PORT=10004 \
        CLUSTER_MANAGER_ADDRESS=127.0.0.1:8080 \
        ETCD_ADDRESSES=127.0.0.1:2379 \
        DATA_PATH=/etc/memdb5 \
        NODE_ID=5 \
		USE_PARTITIONS=true