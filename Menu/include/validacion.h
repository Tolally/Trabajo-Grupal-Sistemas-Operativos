#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;

// Representa un usuario del sistema leído desde el archivo de usuarios.
struct Usuario {
    int id;
    string nombre;
    string username;
    string password;
    string perfil; // ADMIN | GENERAL
};

// Estructura para manejar argumentos de ejecución -u -p -f.
struct Argumentos {
    string usuario;
    string password;
    string archivo; // opcional para conteo de texto
};

// Parsea los argumentos -u (usuario), -p (password) y -f (file). Lanza std::runtime_error si faltan -u o -p.
Argumentos parsearArgumentos(int argc, char** argv);

// Carga usuarios desde el archivo de texto (formato: id;nombre;username;password;perfil).
vector<Usuario> cargarUsuarios(const string& rutaArchivo);

// Busca y autentica al usuario comparando username y password. Devuelve true si coincide y deja el usuario en out.
bool autenticarUsuario(const vector<Usuario>& usuarios, const string& user, const string& pass, Usuario& out);

// Carga el archivo PERFILES.TXT con formato PERFIL;0,1,2,... y devuelve el mapa perfil -> set de opciones permitidas.
unordered_map<string, unordered_set<int>> cargarPerfiles(const string& rutaArchivo);

// Indica si la opción está permitida para el perfil dado utilizando el mapa cargado desde PERFILES.TXT.
bool opcionPermitida(const unordered_map<string, unordered_set<int>>& permisos, const string& perfil, int opcion);
