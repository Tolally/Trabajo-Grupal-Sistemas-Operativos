#include "users.h"

// Carga todos los usuarios desde el archivo indicado, con el id, nombre, usuario, contraseña y tipo de perfil separados por punto y coma.
vector<Usuario> cargarUsuarios(const string& rutaArchivo) {
    vector<Usuario> usuarios;

    ifstream archivo(rutaArchivo);
    
    string linea;
    while (getline(archivo, linea)) {
        if (linea.empty()) continue;

        stringstream ss(linea);
        Usuario u;
        string idStr;
        getline(ss, idStr, ';');
        getline(ss, u.nombre, ';');
        getline(ss, u.username, ';');
        getline(ss, u.password, ';');
        getline(ss, u.perfil, ';');

        try {
            u.id = stoi(idStr);
            usuarios.push_back(u);
        } catch (...) {
            cerr << "Error al convertir ID en línea: " << linea << "\n";
        }
    }

    archivo.close();
    return usuarios;
}

// Guarda todos los usuarios en el archivo txt. Cada usuario con sus detalles se escriben en una línea usando punto y coma como separador.
void guardarUsuarios(const vector<Usuario>& usuarios, const string& rutaArchivo){

    ofstream archivo(rutaArchivo, ios::trunc);

    for (const auto& u : usuarios) {
        archivo << u.id << ";"
                << u.nombre << ";"
                << u.username << ";"
                << u.password << ";"
                << u.perfil << "\n";
    }

    archivo.close();
}

// Crea un nuevo usuario y lo agrega al vector.
void agregarUsuario(vector<Usuario>& usuarios, int id, string nombre, string username, string password, string perfil) {
    Usuario nuevoUsuario; // Crea nuevoUsuario
    // Asigna valores a nuevoUsuario
    nuevoUsuario.id = id;
    nuevoUsuario.nombre = nombre;
    nuevoUsuario.username = username;
    nuevoUsuario.password = password;
    nuevoUsuario.perfil = perfil;

    usuarios.push_back(nuevoUsuario); // Agrega nuevoUsuario a la lista de usuarios
}

// Elimina de la lista el usuario cuyo id coincida.
bool eliminarUsuario(vector<Usuario>& usuarios, int id) {
    // Buscar el usuario por id
    for (size_t i = 0; i < usuarios.size(); ++i) {
        if (usuarios[i].id == id) {
            usuarios.erase(usuarios.begin() + i); // Eliminar usuario
            return true; // Usuario eliminado
        }
    }
    return false; // Usuario no encontrado
}
