#ifndef USERS_H
#define USERS_H

#include <vector>
#include <string>

using namespace std;

struct Usuario {
    int id;
    string nombre;
    string username;
    string password;
    string perfil;
};

vector<Usuario> cargarUsuarios(const string& rutaArchivo);

void guardarUsuarios(const vector<Usuario>& usuarios, const string& rutaArchivo);

void agregarUsuario(vector<Usuario>& usuarios, int id, 
                    string nombre, string username, 
                    string password, string perfil);

bool eliminarUsuario(vector<Usuario>& usuarios, int id);

#endif // USERS_H