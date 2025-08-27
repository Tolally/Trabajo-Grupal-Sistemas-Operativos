#include <iostream>
#include "interface.h"
#include "users.h"

using namespace std;

int main() {
	vector<Usuario> usuarios; // almacenamiento temporal en memoria
	while (true) {
		int opcion = mostrarMenuPrincipal();
		switch (opcion) {
			case 0:
				cout << "Saliendo...\n";
				return;
			case 1:
				pantallaIngresarUsuario(usuarios);
				break;
			case 2:
				pantallaListarUsuarios(usuarios);
				break;
			case 3:
				pantallaEliminarUsuario(usuarios);
				break;
			default:
				cout << "Opción inválida. Intente nuevamente.\n";
				break;
		}
	}
	return 0;
}
