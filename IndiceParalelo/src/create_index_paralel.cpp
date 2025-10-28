#include "../include/create_index_paralel.h"
#include "../include/create_map.h"

using namespace std;

// Alias para el índice
using Index = map<string, map<string,int>>;

// Normaliza una palabra (quita puntuación y pasa a minúsculas)
static string normalizar(const string& token) {
    string out;
    out.reserve(token.size());
    for (unsigned char c : token) {
        if (isalnum(c)) out.push_back(static_cast<char>(tolower(c)));
    }
    return out;
}


// Procesa un lote pero usa mapaLibros (title->id) para escribir id en vez de nombre de archivo.
// Si no encuentra id usa el filename como fallback.
Index crearIndiceInvertidoPorLote(const vector<fs::directory_entry>& lote,
                                  const unordered_map<string,string>& mapaLibros) {
    Index indice;

    for (const auto& entry : lote) {
        if (!entry.is_regular_file()) continue;
        ifstream f(entry.path());
        if (!f.is_open()) continue;

        string filename = entry.path().filename().string();
        string stem = entry.path().stem().string();
        string titleKey = limpiarTitulo(stem);

        // buscar id en el mapa; si no existe usar filename
        string docId;
        auto it = mapaLibros.find(titleKey);
        if (it != mapaLibros.end()) docId = it->second;
        else docId = filename; // fallback

        string palabra;
        while (f >> palabra) {
            string clean = normalizar(palabra);
            if (!clean.empty()) {
                indice[clean][docId]++;
            }
        }
    }

    return indice;
}

// Guarda un índice en disco en el formato simple usado antes.
// Si el archivo ya existe lo sobreescribe.
bool guardarIndice(const string& nombreArchivo, const Index& indice) {
    ofstream out(nombreArchivo, ios::trunc);
    if (!out.is_open()) return false;
    for (const auto& [pal, docs] : indice) {
        out << pal;
        for (const auto& [doc, cnt] : docs) {
            out << ";(" << doc << "," << cnt << ")";
        }
        out << "\n";
    }
    return true;
}
