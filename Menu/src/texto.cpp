#include "../include/texto.h"

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
