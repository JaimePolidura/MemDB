cd ../../../core
sudo ./linux-build.sh

sudo ./run.sh SHOW_DEBUG_LOG=true \
        SERVER_PORT=10000 \
        DATA_PATH=/etc/memdb