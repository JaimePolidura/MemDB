# MemDB

## Setup

- You need an environtment variable called: BOOST_ROOT which points to boost v1.80.0 library's folder, folder name: boost_1_80_0

## How it works

- Each user will authenticate with an authkey. The max length will be 256 characters. It will be specified in config.txt

## Request format
````
                 1 byte
+-------------------------------------+
| Request number... (8 bytes)         | 
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
| Length argument 1º                  | 
+-------------------------------------+
|                 ...                 | 
+-------------------------------------+
| Length argument 2º                  | 
+-------------------------------------+
|                 ...                 | 
+-------------------------------------+
````

## Response format
````
                 1 byte
+-------------------------------------+
| Request number... (8 bytes)         | 
+-------------------------------------+
| Timestamp ... (8 bytes)             |
+-------------------------------------+   
| Error code                    | suc |   
+-------------------------------------+   
| Length response                     | 
+-------------------------------------+   
|                 ...                 | 
+-------------------------------------+
````

## Oplog format
````
             1 byte
+-------------------------------------+
| Opererator number         | f1 | f2 |   
+-------------------------------------+
| Timestamp ... (8 bytes)             |
+------------------------------------ +   
| Length argument 1º                  | 
+-------------------------------------+
|                 ...                 | 
+-------------------------------------+
| Length argument 2º                  | 
+-------------------------------------+
|                 ...                 | 
+-------------------------------------+
| Padding 0000 0000                   |
+-------------------------------------+

````
