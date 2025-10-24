#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include "../include/interface.h"
#include "../include/users.h"

using namespace std;

// Carga variables desde un archivo .env con formato KEY=VALUE por línea.
// Solo asigna variables que no estén ya presentes en el entorno.
static void cargarDotEnvSiExiste() {
    const char* already = getenv("__DOTENV_LOADED");
    if (already && string(already) == "1") return;
    const char* candidatas[] = { ".env", "../.env" };
    for (const char* p : candidatas) {
        ifstream f(p);
        if (!f.is_open()) continue;
        string linea;
        while (getline(f, linea)) {
            if (linea.empty() || linea[0] == '#') continue;
            auto pos = linea.find('=');
            if (pos == string::npos) continue;
            string k = linea.substr(0, pos);
            string v = linea.substr(pos + 1);
            // quitar espacios en extremos
            auto trim = [](string& s){
                while (!s.empty() && isspace(static_cast<unsigned char>(s.front()))) s.erase(s.begin());
                while (!s.empty() && isspace(static_cast<unsigned char>(s.back()))) s.pop_back();
            };
            trim(k); trim(v);
            if (k.empty()) continue;
            if (!getenv(k.c_str())) {
                setenv(k.c_str(), v.c_str(), 0);
            }
        }
        setenv("__DOTENV_LOADED", "1", 1);
        break;
    }
}

// Inicio de la aplicación. Carga los usuarios desde el archivo USUARIOS.txt que se ubica en la ruta definida por USER_FILE (ver el archivo .env) y maneja el ciclo del menú.
int main() {
	// Intenta cargar .env desde cwd o padre si no está en el entorno.
    cargarDotEnvSiExiste();
	const char* envRuta = getenv("USER_FILE");
	if (!envRuta) {
    	cerr << "No se encontró la variable USER_FILE en el entorno.\n";
    return 1;
	}
	string rutaArchivo = envRuta;
	rutaArchivo = rutaArchivo;
	vector<Usuario> usuarios = cargarUsuarios(rutaArchivo);

	// Bucle principal del menú hasta que el usuario decida salir.
	while (true) {
		cout << "==================================\n";
		cout << "   PID del proceso actual: " << getpid() << endl;
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
