#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

using namespace std;

// Representa un usuario del sistema leído desde el archivo de usuarios.
struct UsuarioMP {
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

// Resultado de conteo sobre texto: vocales, consonantes, especiales y palabras.
struct ConteoTexto {
    long vocales = 0;
    long consonantes = 0;
    long especiales = 0;
    long palabras = 0;
};

// Parsea los argumentos -u (usuario), -p (password) y -f (file). Lanza std::runtime_error si faltan -u o -p.
Argumentos parsearArgumentos(int argc, char** argv);

// Carga usuarios desde el archivo de texto (formato: id;nombre;username;password;perfil).
vector<UsuarioMP> cargarUsuarios(const string& rutaArchivo);

// Busca y autentica al usuario comparando username y password. Devuelve true si coincide y deja el usuario en out.
bool autenticarUsuario(const vector<UsuarioMP>& usuarios, const string& user, const string& pass, UsuarioMP& out);

// Carga el archivo PERFILES.TXT con formato PERFIL;0,1,2,... y devuelve el mapa perfil -> set de opciones permitidas.
unordered_map<string, unordered_set<int>> cargarPerfiles(const string& rutaArchivo);

// Indica si la opción está permitida para el perfil dado utilizando el mapa cargado desde PERFILES.TXT.
bool opcionPermitida(const unordered_map<string, unordered_set<int>>& permisos, const string& perfil, int opcion);

// Devuelve true si el texto es palíndromo (ignorando espacios, tildes y mayúsculas/minúsculas y signos); false en caso contrario.
bool esPalindromo(const string& texto);

// Calcula f(x) = x*x + 2x + 8 y devuelve el resultado en double.
double calcularFx(double x);

// Calcula conteo de vocales, consonantes, especiales y palabras sobre el texto indicado.
ConteoTexto contarTexto(const string& texto);

// Lee archivo de texto completo a un string. Devuelve true/false según éxito y deja contenido en out.
bool leerArchivoTexto(const string& ruta, string& outContenido, string& outError);

// Limpia la consola usando el comando del sistema según plataforma.
void limpiarConsola();
