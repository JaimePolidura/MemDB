#include "cluster/clusterdb/ClusterDb.h"

ClusterDb::ClusterDb(configuration_t configuration, logger_t logger) :
    client(configuration->get(ConfigurationKeys::ETCD_ADDRESSES)),
    configuration(configuration),
    logger(logger) {}

void ClusterDb::watchNodeChanges(std::function<void(ClusterDbValueChanged)> onChange) {
    this->nodeChangesWatcher = std::make_unique<etcd::Watcher>(client, "/nodes", [onChange](etcd::Response response){
        onChange(ClusterDbValueChanged::fromEtcdResponse(response));
    }, true);
}

void ClusterDb::setNode(memdbNodeId_t nodeId, node_t node) {
    client.put("/nodes/" + std::to_string(nodeId), Node::toJson(node));
}

node_t ClusterDb::getByNodeId(memdbNodeId_t nodeId) {
    etcd::Value responseValue = this->client.get("/nodes/" + std::to_string(nodeId)).get().value();
    return Node::fromJson(nlohmann::json::parse(responseValue.as_string()));
}

AllNodesResponse ClusterDb::getAllNodes() {
    std::vector<etcd::Value> values = client.ls("/nodes").get().values();
    std::vector<node_t> nodes{};

    std::for_each(values.begin(), values.end(), [&](const etcd::Value& value) {
        nodes.push_back(Node::fromJson(nlohmann::json::parse(value.as_string())));
    });

    return AllNodesResponse{.nodes = nodes};
}

PartitionConfigurationResponse ClusterDb::getPartitionsConfiguration() {
    return PartitionConfigurationResponse {
            .nodesPerPartition = std::stoul(client.get("/partitions/config/nodesPerPartition").get().value().as_string()),
            .maxSize = std::stoul(client.get("/partitions/config/ringSize").get().value().as_string()),
    };
}

RingEntry ClusterDb::getRingEntryByNodeId(memdbNodeId_t nodeId) {
    std::string etcdLink = "/partitions/ring/" + std::to_string(nodeId);
    etcd::Response response = this->client.get(etcdLink).get();

    nlohmann::json responseAsJson = nlohmann::json::parse(response.value().as_string());

    return RingEntry::fromJson(responseAsJson);
}

std::vector<RingEntry> ClusterDb::getRingEntries() {
    std::vector<etcd::Value> values = client.ls("/partitions/ring").get().values();
    std::vector<RingEntry> entries{};

    std::for_each(values.begin(), values.end(), [&entries](const etcd::Value& value) {
        entries.push_back(RingEntry::fromJson(nlohmann::json::parse(value.as_string())));
    });
    std::sort(entries.begin(), entries.end(), [](const RingEntry& A, const RingEntry& B){
        return A.ringPosition < B.ringPosition;
    });

    return entries;
}