#include "cluster/clusterdb/ClusterDb.h"

ClusterDb::ClusterDb(configuration_t configuration, logger_t logger) :
    client(configuration->get(ConfigurationKeys::MEMDB_CORE_ETCD_ADDRESSES)),
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

RingEntry ClusterDb::getRingEntryByNodeId(memdbNodeId_t nodeId) {
    std::string etcdLink = "/partitions/ring/" + std::to_string(nodeId);
    etcd::Response response = this->client.get(etcdLink).get();

    nlohmann::json responseAsJson = nlohmann::json::parse(response.value().as_string());

    return RingEntry::fromJson(responseAsJson);
}