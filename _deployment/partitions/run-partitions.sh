cd ../../
rm -r /etc/memdb1
rm -r /etc/memdb2
rm -r /etc/memdb3
rm -r /etc/memdb4

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

sudo ./run.sh USE_REPLICATION=true \
        SHOW_DEBUG_LOG=true \
        SERVER_PORT=10000 \
        DATA_PATH=/etc/memdb1 \
        NODE_ID=1 \ 
		SEED_NODES=localhost:10000 \
		NODES_PER_PARTITION=2 \
		MAX_PARTITION_SIZE=64 \
		USE_PARTITIONS=true &

sudo ./run.sh USE_REPLICATION=true \
        SHOW_DEBUG_LOG=true \
        SERVER_PORT=10001 \
        DATA_PATH=/etc/memdb2 \
		SEED_NODES=127.0.0.1:10000 \
        NODE_ID=2 \ 
		USE_PARTITIONS=true &

sudo ./run.sh USE_REPLICATION=true \
        SHOW_DEBUG_LOG=true \
        SERVER_PORT=10002 \
        DATA_PATH=/etc/memdb3 \
		SEED_NODES=127.0.0.1:10000 \
        NODE_ID=3 \ 
		USE_PARTITIONS=true &

sudo ./run.sh USE_REPLICATION=true \
        SHOW_DEBUG_LOG=true \
        SERVER_PORT=10003 \
        DATA_PATH=/etc/memdb4 \
		SEED_NODES=127.0.0.1:10000 \		
        NODE_ID=4 \ 
		USE_PARTITIONS=true &

sudo ./run.sh USE_REPLICATION=true \
        SHOW_DEBUG_LOG=true \
        SERVER_PORT=10004 \
        DATA_PATH=/etc/memdb5 \
		SEED_NODES=127.0.0.1:10000 \		
        NODE_ID=5 \ 
		USE_PARTITIONS=true &

sudo ./run.sh USE_REPLICATION=true \
        SHOW_DEBUG_LOG=true \
        SERVER_PORT=10005 \
        DATA_PATH=/etc/memdb6 \
		SEED_NODES=127.0.0.1:10000 \		
        NODE_ID=6 \ 
		USE_PARTITIONS=true &

