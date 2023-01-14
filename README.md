# MemDB

## Setup

- Necesitas variable entorno con llave: BOOST_ROOT y valor: C:\libs\C++\boost. Que apunte a la libreria boost.

## Funcionamiento

- Cada usuario de autentica con una authKey, que enviara al abrir la conexion TCP. Podra tener como maximo 256 caracteres.

## Formato peticion

````
             1 byte
+-------------------------------+
| Request number ... (8 bytes)  | 
+-------------------------------+
| Length auth authKey | f1 | f2 |   
+-------------------------------+   
|             ...               | 
+-------------------------------+
| Opererator number   | f1 | f2 |   
+-------------------------------+   
| Length argument 1º            | 
+-------------------------------+
|             ...               | 
+-------------------------------+
| Length argument 2º            | 
+-------------------------------+
|             ...               | 
+-------------------------------+
````

## Formato respuesta

````
             1 byte
+-------------------------------+
| Request number ... (8 bytes)  |   
+-------------------------------+
| Error code              | suc |   
+-------------------------------+   
| Length response               |
+-------------------------------+
|             ...               | 
+-------------------------------+
````

## Formato persistencia

````
             1 byte
+-------------------------------+
| Timestamp ... (8 bytes)       |   
+-------------------------------+   
| Opererator number   | f1 | f2 |   
+-------------------------------+   
| Length argument 1º            | 
+-------------------------------+
|             ...               | 
+-------------------------------+
| Length argument 2º            | 
+-------------------------------+
|             ...               | 
+-------------------------------+
| Padding 0000 0000             |
+-------------------------------+
| Other                         |
+-------------------------------+

````
