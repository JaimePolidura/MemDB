cd ../../../core

./linux-build.sh

sudo ./run.sh USE_REPLICATION=true \
        SHOW_DEBUG_LOG=true \
        SERVER_PORT=10001 \
        CLUSTER_MANAGER_ADDRESS=127.0.0.1:8080 \
        ETCD_ADDRESSES=127.0.0.1:2379 \
        DATA_PATH=/etc/memdb2 \
        NODE_ID=2
~                                                                                                                                                                                          ~                                         