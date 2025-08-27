#include "users.h"


vector<Usuario> cargarUsuarios(string& rutaArchivo);

void guardarUsuarios(vector<Usuario>& usuarios, string& rutaArchivo);

void agregarUsuario(vector<Usuario>& usuarios, int id, string nombre, string username, string password, string perfil) {
    Usuario nuevoUsuario;
    nuevoUsuario.id = id;
    nuevoUsuario.nombre = nombre;
    nuevoUsuario.username = username;
    nuevoUsuario.password = password;
    nuevoUsuario.perfil = perfil;
    usuarios.push_back(nuevoUsuario);
}

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