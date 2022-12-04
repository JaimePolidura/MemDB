# MemDB

## Setup

- Necesitas variable entorno con llave: BOOST_ROOT y valor: C:\libs\C++\boost. Que apunte a la libreria boost.

## Funcionamiento

- Cada usuario de autentica con una authKey, que enviara al abrir la conexion TCP. Podra tener como maximo 256 caracteres.

## Formato mensaje

````
           1 byte
+---------------------------+
| Length auth authKey | f1 | f2 |   
+---------------------------+   
|           ...             | 
+---------------------------+
````