#ifndef USERS_H
#define USERS_H

#include <vector>
#include <string>

using namespace std;

// Representa un usuario del sistema que debe tener id, nombre, usuario, contraseña y tipo de perfil (admin o general).
struct Usuario {
    int id;
    string nombre;
    string username;
    string password;
    string perfil;
};

// Lee el archivo txt y devuelve un vector con los usuarios.
vector<Usuario> cargarUsuarios(const string& rutaArchivo);

// Escribe todos los usuarios al archivo txt.
void guardarUsuarios(const vector<Usuario>& usuarios, const string& rutaArchivo);

// Agrega un nuevo usuario al vector.
void agregarUsuario(vector<Usuario>& usuarios, int id, 
                    string nombre, string username, 
                    string password, string perfil);

// Elimina el usuario con el id dado.
bool eliminarUsuario(vector<Usuario>& usuarios, int id);

#endif // USERS_H