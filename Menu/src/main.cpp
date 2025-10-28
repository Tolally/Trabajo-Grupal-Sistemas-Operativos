#include "../include/interface.h"
#include "../include/matematicas.h"
#include "../include/validacion.h"
#include "../include/create_index.h"

#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <sys/stat.h>

using namespace std;

// Obtiene una variable de entorno con fallback.
static string getEnvOr(const char* key, const string& fallback) {
    const char* v = getenv(key);
    if (v && *v) return string(v);
    return fallback;
}

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

// Intenta resolver una ruta relativa para que funcione si se ejecuta desde raíz
// o desde la carpeta Menu/. Si no encuentra una alternativa válida, devuelve la original.
static string resolverRuta(const string& path) {
    auto existe = [](const string& p){ struct stat st; return ::stat(p.c_str(), &st) == 0; };
    if (existe(path)) return path;
    // Solo probar alternativas si no es absoluta
    if (!path.empty() && path[0] != '/') {
        // 1) Probar respecto al padre
        string alt1 = string("../") + path;
        if (existe(alt1)) return alt1;
        // 2) Si empieza con "Menu/", probar quitando el prefijo
        const string pref = "Menu/";
        if (path.rfind(pref, 0) == 0) {
            string s = path.substr(pref.size());
            if (existe(s)) return s;
            string alt2 = string("../") + s;
            if (existe(alt2)) return alt2;
        }
    }
    return path;
}

// Muestra un mensaje de autorización denegada y espera Enter.
static void noAutorizado() {
    limpiarConsola();
    cout << "Perfil no autorizado para esta opción.\n";
    cout << "\n(Enter para volver)";
    cin.get();
}

// Inicio de la aplicación MENÚ PRINCIPAL. Maneja argumentos, autenticación y menú.
int main(int argc, char** argv) {
    try {
        // Intenta cargar .env desde cwd o padre si no está en el entorno.
        cargarDotEnvSiExiste();
        Argumentos args = parsearArgumentos(argc, argv);

        // Rutas de archivos desde variables de entorno o valores por defecto
        string rutaUsuarios = resolverRuta(getEnvOr("USER_FILE", "data/USUARIOS.txt"));
        string rutaPerfiles = resolverRuta(getEnvOr("PERFILES_FILE", "data/PERFILES.TXT"));
        string rutaAdmin = resolverRuta(getEnvOr("ADMIN_SYS", ""));
        string rutaParalelo = resolverRuta(getEnvOr("INDICE-INVET-PARALELO",""));

        // Cargar usuarios y autenticar
        auto usuarios = cargarUsuarios(rutaUsuarios);
        if (usuarios.empty()) {
            cerr << "No hay usuarios disponibles. Verifique la ruta: " << rutaUsuarios << "\n";
            return 1;
        }
        Usuario actual;
        // Convertir vector<Usuario> a vector<UsuarioMP>
        std::vector<Usuario> usuariosMP;
        for (const auto& u : usuarios) {
            Usuario ump;
            ump.username = u.username;
            ump.password = u.password;
            ump.perfil = u.perfil;
            usuariosMP.push_back(ump);
        }
        if (!autenticarUsuario(usuariosMP, args.usuario, args.password, actual)) {
            cerr << "Usuario/Password inválidos.\n";
            return 1;
        }

        // Cargar perfiles y permisos
        auto permisos = cargarPerfiles(rutaPerfiles);
        if (permisos.empty()) {
            cerr << "No se pudo cargar PERFILES.TXT. Ruta: " << rutaPerfiles << "\n";
            return 1;
        }

        // Bucle del menú
        while (true) {
            cout << "==============================\n";
            cout << " PID del proceso actual: " << getpid() << endl;
            int opcion = mostrarMenuPrincipal(actual.username, actual.perfil);
            switch (opcion) {
                case 0:
                    cout << "Saliendo...\n";
                    return 0;
                case 1:
                    if (!opcionPermitida(permisos, actual.perfil, 1)) { noAutorizado(); break; }
                    limpiarConsola();
                    system(rutaAdmin.c_str());
                    limpiarConsola();
                    break;
                case 2:
                    limpiarConsola();
                    multiplicarMatrices();
                    break;
                case 3:
                    limpiarConsola();
                    cout << "Juego (en construcción)\n";
                    cout << "\n(Enter para volver)";
                    cin.get();
                    break;
                case 4:
                    pantallaPalindromo();
                    break;
                case 5:
                    pantallaFuncionCuadratica();
                    break;
                case 6:
                    pantallaConteoTexto(args.archivo);
                    break;
                case 7: {
                    limpiarConsola();
                    pantallaCrearIndiceInvertido();
                    break;
                }

                case 8: {
                    limpiarConsola();
                    system(rutaParalelo.c_str());
                    limpiarConsola();
                    break;
                }
                default:
                    limpiarConsola();
                    cout << "Opción inválida. Intente nuevamente.\n";
                    break;
            }
        }
    } catch (const exception& ex) {
        cerr << ex.what() << "\n";
        return 1;
    }
}
