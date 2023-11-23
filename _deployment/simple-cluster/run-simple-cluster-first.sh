cd ../..

# CLUSTER MANAGER
cd ./clustermanager

./build-and-run.sh &
sleep 2s

cd ..

# CLUSTER NODES
cd ./core
sudo ./linux-build.sh

sudo ./run.sh USE_REPLICATION=true \
        SHOW_DEBUG_LOG=true \
        SERVER_PORT=10000 \
        DATA_PATH=/etc/memdb1 \
        NODE_ID=1