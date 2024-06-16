# MemDB
In memory, distributed leaderless key-value store in C++ with partitions, persistence, distributed counters & atomic operations.

## Features
- Basic key-value CRUD operations: get, set, delete & contains.
- Data can be persisted using a log of operations, which is replied when the server boots. This log can be compacted (removing duplicates) and compressed. Every log entry contains the key, value, operation type and a lamport timestamp.
- Data can be distributed over multiple nodes in a leaderless approach. Concurrent writes are detected using lamport clocks since every message in the cluster is timestamped. When a node joins the cluster or reboots, it will receive only the writes that it didn't receive while it was down from any other alive node's log.
- Data can be partitioned over the cluster using consistent hashing approach (similar to DynamoDB). Nodes can join and leave the cluster dynamically.
- Distributed counters.
- Atomic CAS operations over a partition using Paxos.
- Custom binary protocol over TCP.
- Created HTTP Api rest in Go to manage the cluster (./clustermanager). This api exposes operations to get cluster data or kill a node.
- Created Java client.
- The node config can be changed in /etc/memdb
- Every node and client will supply authentication keys when sending request to other nodes.

## Missing features
- SSL/TLS encryption is not supported.
- Configuration cannot be changed in a cluster as it is running.
- Cluster operations (announcing that a node has joined or left a cluster) is not logged. So if a node X has joined the cluster while node A was down, A will never get that information.

## Java client
```java
//Seed nodes are used a first contact nodes. Once the client or a node contacts any of this seed nodes, it will recieve all the other nodes in the cluster.
List<String> seedNodeAddresses = List.of("192.168.1.2:10000", "192.168.1.3:10000", "192.168.1.4:10000");
MemDb memDb = MemDbConnections.cluster("123", seedNodeAddresses);

String slowValue = memDb.quorumGet("key1")
  .awaitQuorum(2, TimeUnit.SECONDS)
  .orElseThrow(() -> new RuntimeException("Quorum not reached"))
  .mostUptoDateResponse();

String fastValue = memDb.get("key1");

while(!memDb.cas("locked", "false", "true")){
  Thread.sleep((long) (Math.random() * 1500) + 100);
}

int counterValue = memDb.getQuorumCounter("counter")
  .awaitQuorum(2, TimeUnit.SECONDS)
  .orElseThrow(() -> new RuntimeException("Quorum not reached"))
  .mostUptoDateResponseAsInt();
```


