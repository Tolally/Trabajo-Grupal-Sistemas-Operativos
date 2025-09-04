#include <iostream>
#include <limits>
#include <algorithm>
#include "interface.h"
#include "users.h"

using namespace std;

namespace {
	// Lee un entero asegurándose que la entrada sea correcta y continúa pidiendo un valor de entrada hasta recibir un número válido
	int leerEnteroSeguro() {
		int v; while (true) {
			if (cin >> v) { cin.ignore(numeric_limits<streamsize>::max(), '\n'); return v; }
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			cout << "Entrada inválida. Intente nuevamente: ";
		}
	}
}

// Interfaz de usuario
int mostrarMenuPrincipal() {
	cout << "==================================\n";
	cout << "   Módulo - Gestión de Usuarios\n";
	cout << "==================================\n";
	cout << "0) Salir\n";
	cout << "1) Ingresar Usuarios\n";
	cout << "2) Listar Usuarios\n";
	cout << "3) Eliminar Usuarios\n";
	cout << "\nOpción : ";
	int op = leerEnteroSeguro();
	return op;
}

// Sección para ingresar un nuevo usuario. Asigna un id, pide los datos y valida el perfil. Si el usuario confirma lo agrega al vector.
void pantallaIngresarUsuario(vector<Usuario>& usuarios) {
	cout << "\n=== Ingreso de usuarios ===\n";
	int id = usuarios.empty() ? 1 : (usuarios.back().id + 1);
    string nombre, username, password, perfil;

    cout << "Id: " << id << '\n';
    cout << "Nombre: "; getline(cin, nombre);
    cout << "Username: "; getline(cin, username);
    cout << "Password: "; getline(cin, password);
	// Validación de perfil
	while (true) {
		cout << "Perfil (ADMIN | GENERAL): "; getline(cin, perfil);
		// quitar espacios
		while (!perfil.empty() && isspace(static_cast<unsigned char>(perfil.front()))) perfil.erase(perfil.begin());
		while (!perfil.empty() && isspace(static_cast<unsigned char>(perfil.back()))) perfil.pop_back();
		for (auto &c : perfil) c = toupper(static_cast<unsigned char>(c));
		if (perfil == "ADMIN" || perfil == "GENERAL") break;
		cout << "Tipo de perfil inválido. Debe ser ADMIN o GENERAL. Intente nuevamente.\n";
	}
	cout << "\n1) Guardar     2) Cancelar\nOpción : ";
	int op = leerEnteroSeguro();
	if (op == 1) {
		agregarUsuario(usuarios, id, nombre, username, password, perfil);
		cout << "Usuario agregado en memoria.\n";
	} else {
		cout << "Operación cancelada.\n";
	}
}

// Muestra una lista de los usuarios. Si no hay usuarios avisa con un mensaje.
void pantallaListarUsuarios(const vector<Usuario>& usuarios) {
	cout << "\n=== Lista de usuarios ===\n";
	cout << "Id   Nombre                 Perfil\n";
	cout << "-----------------------------------\n";
	if (usuarios.empty()) {
		cout << "(Sin usuarios cargados en memoria)\n";
	} else {
		for (const auto& u : usuarios) {
			cout.width(4); cout << u.id; cout << "  ";
			string nombre = u.nombre; 
			if (nombre.size() > 20) nombre = nombre.substr(0,20);
			cout.width(20); cout.setf(ios::left); cout << nombre; cout.unsetf(ios::left);
			cout << ' ' << u.perfil << '\n';
		}
	}
	cout << "\n1) Para Volver : ";
	(void)leerEnteroSeguro();
}

// Permite eliminar un usuario por id con confirmación y muestra una advertencia si se intenta borrar un usuario con perfil ADMIN.
void pantallaEliminarUsuario(vector<Usuario>& usuarios) {
	cout << "\n=== Eliminar Usuarios ===\n";
	if (usuarios.empty()) {
		cout << "No hay usuarios en memoria.\n";
		cout << "1) Volver : ";
		(void)leerEnteroSeguro();
		return;
	}
	cout << "ID usuario a borrar: ";
	int id = leerEnteroSeguro();
	// Buscar usuario por id
	auto it = find_if(usuarios.begin(), usuarios.end(), [&](const Usuario& u){ return u.id == id; });
	if (it == usuarios.end()) {
		cout << "Usuario no encontrado.\n";
		return;
	}
	if (it->perfil == "ADMIN") {
		cout << "ADVERTENCIA: Está intentando eliminar un usuario ADMIN.\n";
	}
	cout << "\n1) Guardar     2) Cancelar\nOpcion : ";
	int op = leerEnteroSeguro();
	if (op != 1) { cout << "Operación cancelada.\n"; return; }
	// Usar función eliminarUsuario para mayor claridad
	if (eliminarUsuario(usuarios, id)) {
		cout << "Usuario eliminado de la lista en memoria.\n";
	} else {
		cout << "Error al eliminar usuario.\n";
	}
}

void ejecutarAplicacion() {
	vector<Usuario> usuarios;
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
}
