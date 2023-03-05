# MemDB Core

This is the main component. This stores the data and handles request & replication

## Setup
- Environtment variable BOOST_ROOT which points to boost v1.80.0 library's folder
- Vcpkg dependencies: etcd-cpp-apiv3

### Windows
- Use MSVC for compiling
- Vcpkg installed on c:

### Linux
- Use gcc for compiling
- To compile use linux-build.sh to automation

## How it works
- Data is stored in a hashmap with fixed number of buckets (default 64). Each bucket will contain an autobalanced AVL Tree.
- It is multithreaded. Each bucket will have a read/write lock.
- It is persistent. When a write comes in, it will be stored in a buffer. When it reaches a threshold, the operations will be appended to a local file. When the server starts up, it will apply all operations stored in the file and compact them.
- Authentication will be carried by a key (AUTH_USER_KEY), which will be present in every request.
- Threads allocated for operations are dynamic. They grow or shrink depending on the demand.
#### Replication
- When a write comes in it will broadcast the write operation to every node. These nodes are queried periodicly to the cluster mamanger.
- To authenticate with the cluster manager a specific auth key will be used (AUTH_CLUSTER_KEY)
- Conflic resolution is handled by LWW (Last write wins) approach. Vector clocks are used to compare events.

## Request format
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
| NodeId (not required)...  (2 bytes) |
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
