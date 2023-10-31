cd ../../../core
sudo ./linux-build.sh

sudo gdb --args ./src/build/src/memdb_run SHOW_DEBUG_LOG=true \
		PERSISTENCE_SEGMENT_SIZE_MB=1 \
        SERVER_PORT=10000 \
        DATA_PATH=/etc/memdb