cd ../../../core
sudo ./linux-build.sh

sudo gdb --args ./src/build/src/memdb_run SHOW_DEBUG_LOG=true \
        SERVER_PORT=10000 \
        DATA_PATH=/etc/memdb