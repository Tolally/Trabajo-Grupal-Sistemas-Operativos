# Entregable 1 - Sistemas Operativos

## Integrantes
- Erick López
- Eduardo Montecinos
- Matias Toledo
- Carlos Duarte

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
./bin/app
```
o altenativamente se puede usar:
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
