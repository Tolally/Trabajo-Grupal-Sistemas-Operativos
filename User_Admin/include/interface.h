#pragma once
#include <vector>
#include "users.h"

using namespace std;

// Muestra el menú principal y devuelve la opción elegida.
int mostrarMenuPrincipal();

// Sección para ingresar un nuevo usuario en memoria.
void pantallaIngresarUsuario(vector<Usuario>& usuarios);

// Mostrar usuarios existentes.
void pantallaListarUsuarios(const vector<Usuario>& usuarios);

// Elimina un usuario de la lista (más una advertencia si es un usuario ADMIN).
void pantallaEliminarUsuario(vector<Usuario>& usuarios);

void ejecutarAplicacion();

void limpiarConsola();
