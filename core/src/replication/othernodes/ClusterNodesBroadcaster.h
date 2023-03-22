#pragma once

#include "shared.h"
#include "utils/datastructures/queue/BlockingQueue.h"
#include "messages/request/Request.h"
#include "messages/request/RequestSerializer.h"
#include "replication/othernodes/ClusterNodesConnections.h"

class ClusterNodesBroadcaster {
private:
    std::shared_ptr<BlockingQueue<Request>> pendingToBroadcast;
    clusterNodesConnections_t clusterNodesConnections;
    std::thread broadcasterTask;
    std::mutex enqueueLock;

public:
    ClusterNodesBroadcaster(clusterNodesConnections_t clusterNodesConnections) :
        clusterNodesConnections(clusterNodesConnections),
        broadcasterTask([this]{this->startBroadcasting();}) {}

    void broadcast(const Request& request) {
        std::unique_lock uniqueLock(this->enqueueLock);

        this->pendingToBroadcast->enqueue(request);
    }

private:
    void startBroadcasting() {
        while(true){
            Request request = this->pendingToBroadcast->dequeue();

            this->clusterNodesConnections->broadcast(request);
        }
    }
};

using clusterNodesBroadcaster_t = std::shared_ptr<ClusterNodesBroadcaster>;