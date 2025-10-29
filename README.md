# Integrantes
- Erick López
- Eduardo Montecinos
- Matias Toledo
- Carlos Duarte

# Entregable 3 - Sistemas Operativos - Grupo 4
Compilar:
```
make -C Menu && make -C IndiceParalelo && make -C User_Admin
```

Ejecutar (por ejemplo):
```
./Menu/bin/pgm -u Tolally -p 12345 -f "libros/010 - Alice's Adventures in Wonderland by Lewis Carroll (48778) (pg11).txt"
```

## Problema 1 - Trabajo sobre el índice invertido
Se agrega al menú principal una opción nueva para crear el indice invertido, con la diferencia de que en esta ocasión el proceso se realiza en paralelo.

Este programa cuenta con las siguientes características:
- Hilos configurables con `N_THREADS` y procesamiento por lotes `N_LOTE`.
- Crea el mapa de libros con formato `id; nombre_libro`.
- El archivo de salida `.idx` utiliza el id del libro en lugar del nombre.
- Registra log del proceso de esta forma: `thread,book,word_count,start_us,end_us`.
- Todos estos archivos creados se guardan en `data/indices/`.

Se agregaron las siguientes variables de entorno al .env:
```
N_THREADS=4
N_LOTE=8
MAPA_LIBROS=data/indices/mapa_libros.txt
INDICE-INVET-PARALELO=IndiceParalelo/bin/indice_paralelo
```

## Problema 2 - Trabajo sobre videojuego
Se implementó un videojuego multijugador con arquitectura cliente-servidor que funciona con sockets. El juego consiste en una carrera por equipos donde los jugadores avanzan por un tablero lanzando dados en turnos. Los equipos compiten para llegar primero a la meta.

Este programa cuenta con las siguientes características:
- Modo multijugador: Múltiples clientes se conectan a un servidor centralizado.
- Juego por equipos: Los jugadores se organizan en equipos.
- Sistema de turnos: Los jugadores tiran dados y avanzan en el tablero de forma alternada.
- Configuración flexible: Tamaño de tablero, dado y otras reglas configurables desde el archivo .env.

Se agregaron las siguientes variables de entorno al .env:
```
GAME_BOARD_SIZE=30        # Tamaño del tablero (número de casillas)
DICE_SIDES=6              # Número de caras del dado
MIN_TEAMS=2               # Mínimo de equipos para iniciar
MIN_PLAYERS_PER_TEAM=1    # Mínimo de jugadores por equipo
MAX_TEAMS=4               # Máximo de equipos permitidos
GAME_PORT=9000            # Puerto para el servidor del juego
GAME_SERVER=bin/server    # Ruta al ejecutable del servidor
GAME_CLIENT=bin/client    # Ruta al ejecutable del cliente
```

# Entregable 2 - Sistemas Operativos - Grupo 4
## Propósito de la aplicación
MENÚ PRINCIPAL con autenticación por argumentos y control de permisos por perfil.

Este programa se inicia con argumentos de ejecución para autenticar un usuario y, según su perfil, habilitar o restringir el uso de opciones del menú. Los usuarios se obtienen del archivo `USUARIOS.txt` archivo de usuarios y los permisos de acceso se leen desde el archivo de perfiles `PERFILES.txt`.

Opciones del menú:

0) Salir
1) Admin usuarios y perfiles (en construcción) - Solo lo pueden usar usuarios de tipo ADMIN
2) Multiplica matrices NxN (en construcción) - Esto se puede usar con el segundo programa llamado `multi`
3) Juego (en construcción)
4) ¿Es palíndromo? - Interfaz para ingresar texto y luego validar o cancelar
5) Calcula f(x) = x^2 + 2x + 8 - Permite ingresar el valor de 'x', y luego muestra el resultado
6) Conteo sobre texto - Muestra cantidad de vocales, consonantes, caracteres especiales y palabras de un texto

## Compilar y ejecutar el programa
Este entregable tiene dos ejecutables: `pgm` (menú principal) y `multi` (multiplicación de matrices).

### 1) Compilar desde la carpeta `Menu/` usando el Makefile:
```
cd Menu
make
```
Esto generará:
- `Menu/bin/pgm`
- `Menu/bin/multi`

### 2) Ejecutar el MENÚ PRINCIPAL (`pgm`) con usuario, contraseña y ruta al archivo de texto:
```
./bin/pgm -u <usuario> -p <contraseña> -f <ruta/archivo.txt>
```

Ejemplo:
```
./bin/pgm -u Tolally -p 12345 -f "libros/010 - Alice's Adventures in Wonderland by Lewis Carroll (48778) (pg11).txt"
```

Salida:
```
==============================
       MENÚ PRINCIPAL
==============================
User: Tolally (ADMIN)

0) Salir
1) Admin Users (En construcción)
2) Multi Matrices NxN
3) Juego
4) ¿Es palíndromo?
5) Calcula f(x) = x^2 + 2x + 8
6) Conteo sobre texto
Opción : 
```

### 3) Ejecutar el programa de MATRICES `multi` (multiplica matrices NxN):
```
./multi "/ruta/a/A.TXT" "/ruta/a/B.TXT" ","
```
Ejemplo
```
./bin/multi ../data/A.txt ../data/B.txt ","
```
Consideraciones para `multi`:
- Lee ambas matrices desde archivos txt con un separador configurable (tercer argumento).
- Además valida que el contenido de las matrices sean números y que la matriz sea de la forma NxN.

## Variables de entorno
Crear un archivo `.env` con lo siguiente:
```
USER_FILE=../data/USUARIOS.txt
PERFILES_FILE=../data/PERFILES.TXT
```


# Entregable 1 - Sistemas Operativos - Grupo 4
## Propósito de la aplicación
Esta aplicación es una herramienta que permite la administración de usuarios. Para ello realiza las siguientes funciones:
1. Ingresar usuarios nuevos.
2. Listar los usuarios que ya existen.
3. Eliminar usuarios por ID.
4. Permitir guardar la información de los usuarios en un archivo txt.

## Compilar y ejecutar el programa
### 1. Ir a la carpeta `User_Admin/` y usar el `Makefile` para compilar:
```
cd User_Admin
make
```
Esto genera un ejecutable en `User_Admin/bin/app`.

### 2. Para ejecutar el programa:
```
make run
```

### 3. Ejemplo de uso 
```
==============================
	Módulo - Gestión de Usuarios
==============================
0) Salir
1) Ingresar Usuarios
2) Listar Usuarios
3) Eliminar Usuarios
Opción : 1

=== Ingreso de usuarios ===
Id: 1
Nombre: Juan Perez
Username: jperez
Password: 1234
Perfil (ADMIN | GENERAL): ADMIN
1) Guardar     2) Cancelar
Opción : 1
Usuario agregado en memoria.
```
Esto genera los siguientes datos dentro del archivo USUARIOS.txt:
```
1;Juan Perez;jperez;1234;ADMIN
```

## Variables de entorno
Es necesario crear un archivo .env en la raiz con la siguiente línea de código:

`USER_FILE = "User_Admin/data/USUARIOS.txt"`

Esto permite leer la información de usuarios.
