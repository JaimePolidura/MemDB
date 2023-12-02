cd ../../../core

# NODE
sudo ./linux-build.sh
sudo gdb --args ./src/build/src/memdb_run USE_REPLICATION=true \
        SHOW_DEBUG_LOG=true \
        PERSISTENCE_SEGMENT_SIZE_MB=1 \
        SERVER_PORT=10000 \
		SEED_NODES=localhost:10000 \
        DATA_PATH=/etc/memdb1 \
        NODE_ID=1