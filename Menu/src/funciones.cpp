#include "funciones.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <stdexcept>

using namespace std;

// Convierte a mayúsculas (ASCII básico) una copia del string.
static string upperCopy(string s) {
    transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return static_cast<char>(toupper(c)); });
    return s;
}

// Normaliza texto para comparación de palíndromos: quita tildes comunes y no alfanuméricos.
static string normalizeForPalindrome(const string& s) {
    auto quitarTilde = [](unsigned char c)->char{
        switch (c) {
            case '\xC3':
                // UTF-8 puede llegar como multibyte; por simplicidad, ignoramos y tratamos ASCII extendido básico
                return ' ';
        }
        // Equivalencias simples para vocales acentuadas típicas en ISO-8859-1/Windows-1252
        // Si llega UTF-8 real, el filtro de alfanuméricos siguiente eliminará bytes no ASCII.
        unsigned char uc = c;
        switch (uc) {
            // minúsculas acentuadas (aprox. en Latin-1)
            case 0xA1: case 0xA2: case 0xA3: case 0xA0: return 'a';
            case 0xA8: return 'e';
            case 0xAD: return 'i';
            case 0xB3: case 0xB2: return 'o';
            case 0xBA: return 'u';
            // mayúsculas acentuadas (aprox.)
            case 0xC0: case 0xC1: case 0xC2: case 0xC3: return 'A';
            case 0xC8: return 'E';
            case 0xCC: return 'I';
            case 0xD2: return 'O';
            case 0xDA: return 'U';
        }
        return static_cast<char>(c);
    };

    string out; out.reserve(s.size());
    for (unsigned char c : s) {
        char q = quitarTilde(c);
        if (isalnum(static_cast<unsigned char>(q))) {
            out.push_back(static_cast<char>(tolower(static_cast<unsigned char>(q))));
        }
    }
    return out;
}

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
vector<UsuarioMP> cargarUsuarios(const string& rutaArchivo) {
    vector<UsuarioMP> usuarios;
    ifstream f(rutaArchivo);
    if (!f.is_open()) {
        return usuarios; // devolver vacío si no existe
    }
    string linea;
    while (getline(f, linea)) {
        if (linea.empty()) continue;
        stringstream ss(linea);
        string idStr; UsuarioMP u;
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
bool autenticarUsuario(const vector<UsuarioMP>& usuarios, const string& user, const string& pass, UsuarioMP& out) {
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

// Devuelve true si el texto es palíndromo (ignorando espacios, tildes y mayúsculas/minúsculas y signos); false en caso contrario.
bool esPalindromo(const string& texto) {
    string n = normalizeForPalindrome(texto);
    size_t i = 0, j = n.size(); if (j > 0) j--; else return true;
    while (i < j) {
        if (n[i] != n[j]) return false;
        ++i; --j;
    }
    return true;
}

// Calcula f(x) = x*x + 2x + 8 y devuelve el resultado en double.
double calcularFx(double x) {
    return x * x + 2.0 * x + 8.0;
}

// Verifica si un carácter es vocal (incluye vocales con tilde comunes en español de forma aproximada)
static bool esVocalChar(unsigned char c) {
    char lc = static_cast<char>(tolower(c));
    const string simples = "aeiou";
    if (simples.find(lc) != string::npos) return true;
    // Aprox. Latin-1 acentuadas frecuentes
    switch (c) {
        case '\xA1': case '\xA2': case '\xA3': case '\xA0': // á variants aprox
        case '\xA8': // é
        case '\xAD': // í
        case '\xB3': case '\xB2': // ó
        case '\xBA': // ú
        case '\xC0': case '\xC1': case '\xC2': case '\xC3': // Á
        case '\xC8': // É
        case '\xCC': // Í
        case '\xD2': // Ó
        case '\xDA': // Ú
            return true;
    }
    return false;
}

// Calcula conteo de vocales, consonantes, especiales y palabras sobre el texto indicado.
ConteoTexto contarTexto(const string& texto) {
    ConteoTexto c;
    bool enPalabra = false;
    for (unsigned char ch : texto) {
        if (isalpha(ch) || ch == '\xD1' || ch == '\xF1') { // incluye Ñ/ñ Latin-1
            if (esVocalChar(ch)) c.vocales++; else c.consonantes++;
            if (!enPalabra) { c.palabras++; enPalabra = true; }
        } else if (isdigit(ch) || isspace(ch)) {
            if (isspace(ch)) enPalabra = false;
        } else {
            c.especiales++;
            enPalabra = false;
        }
    }
    return c;
}

// Lee archivo de texto completo a un string. Devuelve true/false según éxito y deja contenido en out.
bool leerArchivoTexto(const string& ruta, string& outContenido, string& outError) {
    if (ruta.empty()) { outError = "Ruta vacía"; return false; }
    ifstream f(ruta);
    if (!f.is_open()) { outError = "No se pudo abrir el archivo"; return false; }
    stringstream ss; ss << f.rdbuf();
    outContenido = ss.str();
    return true;
}

// Limpia la consola usando el comando del sistema según plataforma.
void limpiarConsola() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}
