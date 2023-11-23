cd ../../../core

sudo ./linux-build.sh

sudo ./src/build/src/memdb_run USE_REPLICATION=true \
        SHOW_DEBUG_LOG=true \
        SERVER_PORT=10004 \
		SEED_NODES=127.0.0.1:10000 \
        DATA_PATH=/etc/memdb5 \
        NODE_ID=5 \
		USE_PARTITIONS=true