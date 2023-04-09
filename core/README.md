# MemDB Core

This is the main component. This stores the data and handles request & replication.

## How it works
- Data is stored in a hashmap with fixed number of buckets (default 64). Each bucket will contain an autobalanced AVL Tree and a shared lock to handle concurrency.
- Data is persistent. When a write comes in, it will be stored in a buffer. When it reaches a threshold, the operations will be appended to a local file. When the server starts up, it will apply all operations stored in the file and compact them.
- TCP protocol is used for serve request.
- Authentication will be carried by a key (AUTH_USER_KEY), which will be present in every request.
- Threads allocated for operations are dynamic. They grow or shrink depending on the demand.
- Every operation that can be executed in the database (set, delete, get etc.) will be indentified by a operatorNumber, which will be in every request.
#### Replication
- When a write from a client is executed successfuly, the server will broadcast the write operation to every node in the cluster.
- Conflict resolution in replication is handled by LWW (Last write wins) approach. Every replication request & data stored will have a Lamport clock. If a SET request comes in with a lower timestamp than the stored data, it will get rejected.
- The cluster information is stored in etcd. Every node watch for changes in etcd nodes list, so that it can update its node list.
- When a node fails and restarts. It will ask a random node to send him all the operations since the last timestamp that he processed (stored in the operation log).
- When a new node joins the cluster. It will ask a random node to send him all operations.
- To authenticate with the cluster manager adnd the nodes in the cluster, a specific auth key will be used (AUTH_CLUSTER_KEY)
- Every node can have three states: RUNNING, SHUTDOWNm BOOTING

### Configuration
Configuration keys are stored in environtment variables:
- AUTH_USER_KEY. Used to authenticate clients. Defautl: 123
- SERVER_MIN_THREADS. Min nº of allocated threads that will handle TCP connections. Default: 20
- SERVER_MAX_THREADS. Max nº of allocated threads that will handle TCP connections. Default: 100
- PORT. Default: 10000
- NUMBER_BUCKETS. Internal hashmap number of buckets, each bucket will contain a shared lock and AVL Tree. Default: 64
- PERSISTANCE_WRITE_EVERY. Threshold of the wrrites operation log buffer used for persistence. Default: 50
- USE_REPLICATION. Default: false.
- AUTH_CLUSTER_KEY. Used for authenticate to cluster nodes and the cluster manager. Default: abc
- ETCD_ADDRESSES. Etcd addresses. Default: 127.0.0.1:2379. Exmaple <address1>,<address2> etc.
- CLUSTER_MANAGER_ADDRESS. Default: 127.0.0.1:8080
- CLUSTER_MANAGER_ADDRESS_USING_DNS. If set to true a DNS lookup will be executed when connecting to the cluster manager. Default: false.
- NODE_ID. The actual node id for the node itself. Default: 1.

### Operations
## Available operators
| **Name**    | **Operator number** | **Args**   | **Auth key type required** |
|-------------|---------------------|------------|----------------------------|
| SET         | 0x01                | Key, value | AUTH_USER_KEY              |
| GET         | 0x02                | Key        | AUTH_USER_KEY              |
| DELETE      | 0x03                | Key        | AUTH_USER_KEY              |
| HEATH_CHECK | 0x04                |            | AUTH_CLUSTER_KEY           |
| SYNC_DATA   | 0x05                | Timestamp  | AUTH_CLUSTER_KEY           |

## Error codes
| **Name**           | **Error code** | **Desc**                                                                                                 |
|--------------------|----------------|----------------------------------------------------------------------------------------------------------|
| AUTH_ERROR         | 0x00           | Authenticaion is invalid                                                                                 |
| UNKNOWN_KEY        | 0x01           | A key hasnt been found. Ex when you send delete request, and the key is not in the map                   |
| UNKNOWN_OPERATOR   | 0x02           | Operator number not found                                                                                |
| ALREADY_REPLICATED | 0x03           | Replication request has got rejected because of a conflict (only used in replication)                    |
| NOT_AUTHORIZED     | 0x04           | Someone uses a different auth key to execute an operator, Ex client trying to execute SYNC_DATA          |
| INVALID_NODE_STATE | 0x05           | Request is sended to a node that can't execute request due to its state, exmaple: when state is SHUTDOWN |


## Request format
- The binary format is bigendian

````
                 1 byte
+-------------------------------------+
| Total request length...   (4 bytes) |                 
+-------------------------------------+
| Request number...         (4 bytes) | 
+-------------------------------------+
| Length auth authKey       | f1 | f2 |   
+-------------------------------------+   
|                 ...                 | 
+-------------------------------------+
| Opererator number         | f1 | f2 |   
+-------------------------------------+
| Timestamp ... (8 bytes)             |
+-------------------------------------+
| NodeId (only replication) (2 bytes) |
+------------------------------------ +   
| Length argument 1º ...    (4 bytes) |
+-------------------------------------+
|                 ...                 | 
+-------------------------------------+
| Length argument 2º ...    (4 bytes) | 
+-------------------------------------+
|                 ...                 | 
+-------------------------------------+
````

## Response format
````
                 1 byte
+-------------------------------------+
| Request number...         (4 bytes) | 
+-------------------------------------+
| Timestamp ...             (8 bytes) |
+-------------------------------------+   
| Error code                    | suc |   
+-------------------------------------+   
| Length response ...       (4 bytes) |
+-------------------------------------+   
|                 ...                 | 
+-------------------------------------+
````

## Operation persistence format
````
             1 byte
+-------------------------------------+
| Opererator number         | f1 | f2 |   
+-------------------------------------+
| Timestamp ...             (8 bytes) |
+-------------------------------------+   
| Length argument 1º ...    (4 bytes) |
+-------------------------------------+
|                 ...                 | 
+-------------------------------------+
| Length argument 2º ...    (4 bytes) | 
+-------------------------------------+
|                 ...                 | 
+-------------------------------------+
| Padding 0000 0000         (4 bytes) |
+-------------------------------------+

````
