cd ../../..

# nodesPerPartition "2"
# ringSize "64"

# "{\"nodeId\":\"1\",\"ringPosition\": 2}"
# "{\"nodeId\":\"4\",\"ringPosition\": 29}"
# "{\"nodeId\":\"2\",\"ringPosition\": 35}"
# "{\"nodeId\":\"3\",\"ringPosition\": 62}"

# "{\"nodeId\":\"1\",\"address\":\"127.0.0.1:10000\",\"state\":\"BOOTING\"}"
# "{\"nodeId\":\"2\",\"address\":\"127.0.0.1:10001\",\"state\":\"BOOTING\"}"
# "{\"nodeId\":\"3\",\"address\":\"127.0.0.1:10002\",\"state\":\"BOOTING\"}"
# "{\"nodeId\":\"4\",\"address\":\"127.0.0.1:10003\",\"state\":\"BOOTING\"}"

# CLUSTER NODES
cd ./core
sudo ./linux-build.sh

sudo gdb --args ./src/build/src/memdb_run USE_REPLICATION=true \
        SHOW_DEBUG_LOG=true \
        ADDRESS=192.168.1.159 \
        SERVER_PORT=10000 \
        SEED_NODES=192.168.1.159:10000 \
        DATA_PATH=/etc/memdb1 \
        NODE_ID=1 \
        NODES_PER_PARTITION=2 \
        MAX_PARTITION_SIZE=64 \
        USE_PARTITIONS=true