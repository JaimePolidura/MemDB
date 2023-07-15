#include "cluster/clusterdb/ClusterDb.h"

void ClusterDb::watchNodeChanges(std::function<void(ClusterDbValueChanged)> onChange) {
    this->nodeChangesWatcher = std::make_unique<etcd::Watcher>(client, "/nodes", [onChange](etcd::Response response){
        onChange(ClusterDbValueChanged::fromEtcdResponse(response));
    }, true);
}

void ClusterDb::setNode(memdbNodeId_t nodeId, node_t node) {
    client.put("/nodes/" + std::to_string(nodeId), Node::toJson(node));
}

RingEntry ClusterDb::getRingEntryByNodeId(memdbNodeId_t nodeId) {
    etcd::Response response = this->client.get("/partitions/ring/" + nodeId).get();
    nlohmann::json responseAsJson = nlohmann::json::parse(response.value().as_string());

    return RingEntry::fromJson(responseAsJson);
}