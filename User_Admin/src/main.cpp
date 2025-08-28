#include <iostream>
#include <cstdlib>
#include "interface.h"
#include "users.h"

using namespace std;

// Inicio de la aplicación. Carga los usuarios desde el archivo USUARIOS.txt que se ubica en la ruta definida por USER_FILE (ver el archivo .env) y maneja el ciclo del menú.
int main() {

	const char* envRuta = getenv("USER_FILE");
	if (!envRuta) {
    	cerr << "No se encontró la variable USER_FILE en el entorno.\n";
    return 1;
	}
	string rutaArchivo = envRuta;
	cout << "Ruta archivo: " << rutaArchivo << endl;
	vector<Usuario> usuarios = cargarUsuarios(rutaArchivo);

	// Bucle principal del menú hasta que el usuario decida salir.
	while (true) {
		int opcion = mostrarMenuPrincipal();
		switch (opcion) {
			case 0:
				cout << "Saliendo...\n";
				guardarUsuarios(usuarios, rutaArchivo);
				return 0;
			case 1:
				pantallaIngresarUsuario(usuarios);
				guardarUsuarios(usuarios, rutaArchivo);
				break;
			case 2:
				pantallaListarUsuarios(usuarios);
				break;
			case 3:
				pantallaEliminarUsuario(usuarios);
				guardarUsuarios(usuarios, rutaArchivo);
				break;
			default:
				cout << "Opción inválida. Intente nuevamente.\n";
				break;
		}
	}
	return 0;
}
