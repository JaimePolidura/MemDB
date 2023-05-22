# MemDB Core

This is the main component. This stores the data and handles request & replication.

## How it works
- Data is stored in a hashmap with fixed number of buckets (default 64). Each bucket will contain an autobalanced AVL Tree and a shared lock to handle concurrency.
- Data is persistent. When a append comes in, it will be stored in a buffer. When it reaches a threshold, the operations will be appended to a local file. When the server starts up, it will apply all operations stored in the file and compact them.
- TCP protocol is used for serve request.
- Authentication will be carried by different keys depending on the context. To use the memdb as a user, you will need AUTH_API_KEY.
- Threads allocated for operations are dynamic. They grow or shrink depending on the demand.
- Every operation that can be executed in the database (set, delete, get etc.) will be indentified by a operatorNumber, which will be in every request.
#### Replication
- When a append from a client is executed successfuly, the server will broadcast the append operation to every node in the cluster.
- Conflict resolution in replication is handled by LWW (Last append wins) approach. Every replication request & data stored will have a Lamport clock. If a SET request comes in with a lower timestamp than the stored data, it will get rejected.
- The cluster information is stored in etcd. Every node watchNodeChanges for changes in etcd nodes list, so that it can update its node list.
- When a node fails and restarts. It will ask a random node to send him all the operations since the last timestamp that he processed (stored in the operation log).
- When a new node joins the cluster. It will ask a random node to send him all operations.
- Every node can have three states: RUNNING, SHUTDOWNm BOOTING

### Configuration
Configuration keys are stored in environtment variables:
- MEMDB_CORE_AUTH_NODE_KEY. Used by nodes to authenticate to each other. Example: sync their oplog, replicate etc. Default: 123
- MEMDB_CORE_AUTH_MAINTENANCE_KEY. Used to authenticate the cluster manager. Example: health checks. Default: 456
- MEMDB_CORE_AUTH_API_KEY. Used by external users to authenticate, this will be used on your app to connect to the database. Default: 789
- MEMDB_CORE_SERVER_MIN_THREADS. Min nº of allocated threads that will handle request. Default: 20
- MEMDB_CORE_SERVER_MAX_THREADS. Max nº of allocated threads that will handle request. Default: 100
- MEMDB_CORE_PORT. Default: 10000
- MEMDB_CORE_NUMBER_BUCKETS. Internal hashmap number of buckets, each bucket will contain a shared lock and AVL Tree. Default: 64
- MEMDB_CORE_PERSISTANCE_WRITE_EVERY. Threshold of the wrrites operation log buffer used for persistence. Default: 50
- MEMDB_CORE_USE_REPLICATION. Default: false.
- MEMDB_CORE_ETCD_ADDRESSES. Etcd addresses. Default: 127.0.0.1:2379. Exmaple <address1>,<address2> etc.
- MEMDB_CORE_CLUSTER_MANAGER_ADDRESS. Default: 127.0.0.1:8080
- MEMDB_CORE_NODE_ID. The actual node id for the node itself. Default: 1.
- MEMDB_CORE_SHOW_DEBUG_LOG. Default false

### Operations
## Available operators
| **Name**    | **Operator number** | **Args**   | **Auth key type required**      |
|-------------|---------------------|------------|---------------------------------|
| SET         | 0x01                | Key, value | AUTH_NODE_KEY, AUTH_API_KEY     |
| GET         | 0x02                | Key        | AUTH_API_KEY                    |
| DELETE      | 0x03                | Key        | AUTH_NODE_KEY, AUTH_API_KEY     |
| HEATH_CHECK | 0x04                |            | MEMDB_CORE_AUTH_MAINTENANCE_KEY |
| SYNC_OPLOG  | 0x05                | Timestamp  | MEMDB_CORE_AUTH_NODE_KEY        |

## Error codes
| **Name**           | **Error code** | **Desc**                                                                                                 |
|--------------------|----------------|----------------------------------------------------------------------------------------------------------|
| AUTH_ERROR         | 0x00           | Authenticaion is invalid                                                                                 |
| UNKNOWN_KEY        | 0x01           | A key hasnt been found. Ex when you send delete request, and the key is not in the map                   |
| UNKNOWN_OPERATOR   | 0x02           | Operator number not found                                                                                |
| ALREADY_REPLICATED | 0x03           | Replication request has got rejected because of a conflict (only used in replication)                    |
| NOT_AUTHORIZED     | 0x04           | Someone uses a different auth key to execute an operator, Ex AUTH_API_KEY trying to execute SYNC_DATA    |
| INVALID_NODE_STATE | 0x05           | Request is sended to a node that can't execute request due to its state, exmaple: when state is SHUTDOWN |

## Request format
- The binary format is bigendian
- If flag1 is 1 in Auth byte, the request has a nodeId

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
| Total response length...  (4 bytes) |                 
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
