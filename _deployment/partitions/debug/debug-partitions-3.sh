cd ../../../core

sudo ./linux-build.sh

sudo gdb --args ./src/build/src/memdb_run USE_REPLICATION=true \
        SHOW_DEBUG_LOG=true \
        SERVER_PORT=10002 \
		SEED_NODES=127.0.0.1:10000 \
        DATA_PATH=/etc/memdb3 \
        NODE_ID=3 \
		USE_PARTITIONS=true