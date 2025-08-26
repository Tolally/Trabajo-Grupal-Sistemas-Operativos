#include <iostream>
#include <limits>
#include <algorithm>
#include "interface.h"

using namespace std;

namespace {
	int leerEnteroSeguro() {
		int v; while (true) {
			if (cin >> v) { cin.ignore(numeric_limits<streamsize>::max(), '\n'); return v; }
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			cout << "Entrada inválida. Intente nuevamente: ";
		}
	}

	string leerLinea(const string& etiqueta) {
		cout << etiqueta;
		string s; getline(cin, s); return s;
	}
}

// Interfaz de usuario
int mostrarMenuPrincipal() {
	cout << "==============================\n";
	cout << "   Módulo - Gestión de Usuarios\n";
	cout << "==============================\n";
	cout << "0) Salir\n";
	cout << "1) Ingresar Usuarios\n";
	cout << "2) Listar Usuarios\n";
	cout << "3) Eliminar Usuarios\n";
	cout << "\nOpción : ";
	int op = leerEnteroSeguro();
	return op;
}

void pantallaIngresarUsuario(vector<Usuario>& usuarios) {
	cout << "\n=== Ingreso de usuarios ===\n";
	Usuario u{};
	u.id = usuarios.empty() ? 1 : (usuarios.back().id + 1); // provisional
	cout << "Id: " << u.id << '\n';
	cout << "Nombre: "; getline(cin, u.nombre);
	cout << "Username: "; getline(cin, u.username);
	cout << "Password: "; getline(cin, u.password);
	// Validación de perfil
	while (true) {
		cout << "Perfil (ADMIN | GENERAL): "; getline(cin, u.perfil);
		// quitar espacios
		while (!u.perfil.empty() && isspace(static_cast<unsigned char>(u.perfil.front()))) u.perfil.erase(u.perfil.begin());
		while (!u.perfil.empty() && isspace(static_cast<unsigned char>(u.perfil.back()))) u.perfil.pop_back();
		for (auto &c : u.perfil) c = toupper(static_cast<unsigned char>(c));
		if (u.perfil == "ADMIN" || u.perfil == "GENERAL") break;
		cout << "Tipo de perfil inválido. Debe ser ADMIN o GENERAL. Intente nuevamente.\n";
	}
	cout << "\n1) Guardar     2) Cancelar\nOpción : ";
	int op = leerEnteroSeguro();
	if (op == 1) {
		// Solo agregamos en memoria temporal (no persistente por ahora)
		usuarios.push_back(u);
		cout << "Usuario agregado en memoria.\n";
	} else {
		cout << "Operación cancelada.\n";
	}
}

void pantallaListarUsuarios(const vector<Usuario>& usuarios) {
	cout << "\n=== Lista de usuarios ===\n";
	cout << "Id   Nombre                 Perfil\n";
	cout << "-----------------------------------\n";
	if (usuarios.empty()) {
		cout << "(Sin usuarios cargados en memoria)\n";
	} else {
		for (const auto& u : usuarios) {
			cout.width(4); cout << u.id; cout << "  ";
			string nombre = u.nombre; if (nombre.size() > 20) nombre = nombre.substr(0,20);
			cout.width(20); cout.setf(ios::left); cout << nombre; cout.unsetf(ios::left);
			cout << ' ' << u.perfil << '\n';
		}
	}
	cout << "\n1) Para Volver : ";
	(void)leerEnteroSeguro();
}

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
	usuarios.erase(it);
	cout << "Usuario eliminado de la lista en memoria.\n";
}

void ejecutarAplicacion() {
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
}

