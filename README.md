# Integrantes
- Erick López
- Eduardo Montecinos
- Matias Toledo
- Carlos Duarte

# Entregable 4 - Sistemas Operativos - Grupo 4
Borrar bin´s:
```
make -C Menu clean && make -C IndiceParalelo clean && make -C User_Admin clean && make -C Buscador clean && make -C Analisis_estadistica clean
```

Compilar todo:
```
make -C Menu && make -C IndiceParalelo && make -C User_Admin && make -C Buscador && make -C Analisis_estadistica
```

Ejecutar (por ejemplo):
```
./Menu/bin/pgm -u Tolally -p 12345 -f "libros/010 - Alice's Adventures in Wonderland by Lewis Carroll (48778) (pg11).txt"
```

## Problema 1 - Buscador
Se implementó un sistema de búsqueda sobre el índice invertido con arquitectura de 3 capas usando sockets:

```
┌─────────────┐     socket     ┌─────────────┐     socket     ┌─────────────┐
│  Buscador   │ ────────────►  │    Cache    │ ────────────►  │    Motor    │
│  (cliente)  │ ◄────────────  │  (servidor) │ ◄────────────  │  (servidor) │
└─────────────┘    respuesta   └─────────────┘    respuesta   └─────────────┘
```

### Componentes:
- **Buscador**: Cliente que envía consultas al Cache y muestra la respuesta JSON.
- **Cache**: Servidor intermedio que almacena búsquedas previas. Si tiene el resultado lo devuelve (HIT), si no consulta al Motor (MISS).
- **Motor de Búsqueda**: Servidor que carga el índice invertido, busca la palabra y devuelve los TOP K resultados ordenados por frecuencia.

### Características:
- Comunicación entre procesos mediante sockets TCP.
- Cache con tamaño configurable (`CACHE_SIZE`).
- Motor devuelve los TOP K resultados (`TOPK`).
- Respuesta en formato JSON con: query, origen (cache/motor), tiempos, y lista de libros con score.
- El Motor hace mapeo inverso de ID a nombre real del libro usando el archivo MAPA_LIBROS.
- El Buscador muestra su PID y la respuesta completa.

### Formato de respuesta JSON:
```json
{
  "query": "anillo",
  "origen_respuesta": "cache",
  "tiempo_cache_us": 1,
  "tiempo_motor_us": 0,
  "tiempo_total_us": 1,
  "palabras_buscadas": 1,
  "palabras_encontradas": 1,
  "topK": 3,
  "respuesta": [
    {"libro": "El señor de los anillos", "score": 3},
    {"libro": "Millitas", "score": 2},
    {"libro": "Ojos rojos", "score": 1}
  ]
}
```

### Variables de entorno:
```
CACHE_PORT=8080
MOTOR_PORT=8081
CACHE_SIZE=100
TOPK=3
INDICE_PATH=data/indices/indice.idx
MAPA_LIBROS=data/indices/mapa_libros.txt
```

### Compilar y ejecutar:
```bash
# Compilar
make -C Buscador

# Ejecutar (en 3 terminales separadas):
# Terminal 1 - Motor (primero)
./Buscador/bin/motor

# Terminal 2 - Cache (segundo)
./Buscador/bin/CACHE

# Terminal 3 - Buscador (Opcion 9 en Menu Principal)
./Menu/bin/pgm -u Tolally -p 12345 -f "libros/010 - Alice's Adventures in Wonderland by Lewis Carroll (48778) (pg11).txt"
```

## Problema 2 - Análisis de rendimiento y estadísticas de juegos

Este problema se divide en dos partes:

### Parte A) Análisis de rendimiento del trabajo con threads

Se creó un programa externo que permite ejecutar múltiples veces el índice invertido paralelo con diferentes cantidades de threads y registrar los tiempos de ejecución.

#### Características:
- Nueva opción de menú que llama al programa de análisis.
- Permite configurar un arreglo de `CANT_THREADS` (ej: `[1, 2, 4, 8]`).
- Realiza múltiples ejecuciones del índice invertido con cada configuración.
- Registra el tiempo de ejecución en un archivo log con formato: `(CANT_THREADS, tiempo_ms)`.
- Al finalizar, llama a un script Python que genera un gráfico de rendimiento (tiempo vs threads).
- El gráfico se guarda como imagen en una carpeta configurable (no se muestra en pantalla).

#### Variables de entorno:
```
CANT_THREADS=[1,2,4,8]
RENDIMIENTO_LOG=data/logs/rendimiento.csv
GRAFICOS_PATH=data/graficos/
```

#### Script Python (`analisis_rendimiento.py`):
```bash
python3 scripts/analisis_rendimiento.py
```
Genera: `data/graficos/rendimiento_threads.png`

---

### Parte B) Generación de estadísticas del juego

Se creó un programa en Python que lee el log del juego (entrega anterior) y genera 4 gráficos estadísticos.

#### Estadísticas implementadas:
1. **Jugadores por equipo por partida**: Gráfico de barras agrupadas.
2. **Posición final promedio por equipo**: Gráfico de barras.
3. **Distribución de tiradas de dado**: Histograma.
4. **Tiempo promedio por turno por jugador**: Gráfico de líneas.

#### Características:
- Lee el archivo de log del juego como base de datos.
- Genera los 4 gráficos de una sola vez.
- Guarda las imágenes en una carpeta específica (variable de entorno).
- No muestra los gráficos en pantalla.

#### Variables de entorno:
```
GAME_LOG=data/logs/game_log.csv
ESTADISTICAS_PATH=data/graficos/juego/
```

#### Ejecutar:
```bash
python3 scripts/estadisticas_juego.py
```

Genera:
- `data/graficos/juego/jugadores_por_equipo.png`
- `data/graficos/juego/posicion_final_equipos.png`
- `data/graficos/juego/distribucion_dado.png`
- `data/graficos/juego/tiempo_por_turno.png`

---

# Entregable 3 - Sistemas Operativos - Grupo 4


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
