#include "replication/clusterdb/ClusterDbValueChanged.h"
#include "replication/Node.h"
#include "replication/othernodes/ClusterNodesConnections.h"

class ClusterDbNodeChangeHandler {
private:
    clusterNodesConnections_t clusterNodeConnections;

public:
    ClusterDbNodeChangeHandler(clusterNodesConnections_t clusterNodeConnections): clusterNodeConnections(clusterNodeConnections) {}

    ClusterDbNodeChangeHandler() = default;

    void handleChange(const Node& newNode, const ClusterDbChangeType changeType) {
        if(changeType == ClusterDbChangeType::PUT)
            this->updateNodes(newNode);
        else if (changeType == ClusterDbChangeType::DELETED)
            this->deleteNode(newNode);
    }

private:
    void updateNodes(const Node &node) {
        auto existsByNodeId = this->clusterNodeConnections->existsByNodeId(node.nodeId);

        if(existsByNodeId)
            this->clusterNodeConnections->replaceNode(node);
        else
            this->clusterNodeConnections->addNode(node);
    }

    void deleteNode(const Node &node) {
        this->clusterNodeConnections->deleteNodeById(node.nodeId);
    }
};