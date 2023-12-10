cd ../../../core

sudo ./linux-build.sh

sudo gdb --args ./src/build/src/memdb_run USE_REPLICATION=true \
        SHOW_DEBUG_LOG=true \
		ADDRESS=192.168.1.159 \
        SERVER_PORT=10001 \
		SEED_NODES=192.168.1.159:10000 \
        DATA_PATH=/etc/memdb2 \
        NODE_ID=2 \
		USE_PARTITIONS=true