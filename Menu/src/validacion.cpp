#include "../include/validacion.h"

// Declaración anticipada
static string upperCopy(string s);

// Parsea los argumentos -u (usuario), -p (password) y -f (file). Lanza std::runtime_error si faltan -u o -p.
Argumentos parsearArgumentos(int argc, char** argv) {
    Argumentos args;
    for (int i = 1; i < argc; ++i) {
        string a = argv[i];
        if (a == "-u" && i + 1 < argc) {
            args.usuario = argv[++i];
        } else if (a == "-p" && i + 1 < argc) {
            args.password = argv[++i];
        } else if (a == "-f" && i + 1 < argc) {
            args.archivo = argv[++i];
        } else {
            // Ignorar argumentos desconocidos para robustez
        }
    }
    if (args.usuario.empty() || args.password.empty()) {
        throw runtime_error("Uso: pgm -u <usuario> -p <password> [-f <archivo_texto>]");
    }
    return args;
}

// Carga usuarios desde el archivo de texto (formato: id;nombre;username;password;perfil).
vector<Usuario> cargarUsuarios(const string& rutaArchivo) {
    vector<Usuario> usuarios;
    ifstream f(rutaArchivo);
    if (!f.is_open()) {
        return usuarios; // devolver vacío si no existe
    }
    string linea;
    while (getline(f, linea)) {
        if (linea.empty()) continue;
        stringstream ss(linea);
        string idStr; Usuario u;
        getline(ss, idStr, ';');
        getline(ss, u.nombre, ';');
        getline(ss, u.username, ';');
        getline(ss, u.password, ';');
        getline(ss, u.perfil, ';');
        try {
            u.id = stoi(idStr);
            if (!u.username.empty()) usuarios.push_back(u);
        } catch (...) {
            // línea inválida, la ignoramos
        }
    }
    return usuarios;
}

// Busca y autentica al usuario comparando username y password. Devuelve true si coincide y deja el usuario en out.
bool autenticarUsuario(const vector<Usuario>& usuarios, const string& user, const string& pass, Usuario& out) {
    for (const auto& u : usuarios) {
        if (u.username == user && u.password == pass) { out = u; return true; }
    }
    return false;
}

// Carga el archivo PERFILES.TXT con formato PERFIL;0,1,2,... y devuelve el mapa perfil -> set de opciones permitidas.
unordered_map<string, unordered_set<int>> cargarPerfiles(const string& rutaArchivo) {
    unordered_map<string, unordered_set<int>> mapa;
    ifstream f(rutaArchivo);
    if (!f.is_open()) return mapa;
    string linea;
    while (getline(f, linea)) {
        if (linea.empty()) continue;
        auto pos = linea.find(';');
        if (pos == string::npos) continue;
        string perfil = linea.substr(0, pos);
        string lista = linea.substr(pos + 1);
        unordered_set<int> opts;
        string num;
        stringstream ss(lista);
        while (getline(ss, num, ',')) {
            try { opts.insert(stoi(num)); } catch (...) {}
        }
        mapa[upperCopy(perfil)] = move(opts);
    }
    return mapa;
}

// Indica si la opción está permitida para el perfil dado utilizando el mapa cargado desde PERFILES.TXT.
bool opcionPermitida(const unordered_map<string, unordered_set<int>>& permisos, const string& perfil, int opcion) {
    auto it = permisos.find(upperCopy(perfil));
    if (it == permisos.end()) return false;
    return it->second.count(opcion) > 0;
}

// Convierte a mayúsculas (ASCII básico) una copia del string.
static string upperCopy(string s) {
    transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return static_cast<char>(toupper(c)); });
    return s;
}
