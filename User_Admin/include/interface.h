#pragma once
#include <vector>
#include "users.h"

using namespace std;

int mostrarMenuPrincipal();

void pantallaIngresarUsuario(vector<Usuario>& usuarios);

void pantallaListarUsuarios(const vector<Usuario>& usuarios);

void pantallaEliminarUsuario(vector<Usuario>& usuarios);

void ejecutarAplicacion();
