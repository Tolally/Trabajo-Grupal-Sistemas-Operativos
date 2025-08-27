#include <iostream>
#include "interface.h"
#include "users.h"

using namespace std;

int main(int argc, char **argv) {
	
	vector<Usuario> usuarios = {
    {1, "Juan Perez", "jperez", "1234", "ADMIN"},
    {2, "Ana Gomez", "agomez", "abcd", "GENERAL"},
    {3, "Luis Torres", "ltorres", "pass", "GENERAL"}
	};

	while (true) {
		int opcion = mostrarMenuPrincipal();
		switch (opcion) {
			case 0:
				cout << "Saliendo...\n";
				return 0;
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
