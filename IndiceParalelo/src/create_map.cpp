#include "../include/create_map.h"

using namespace std;

// Elimina prefijo numérico y separadores comunes ("001 - ", "01_", "12. ", etc.)
string limpiarTitulo(const string& name) {
    static const regex pref(R"(^\s*\d+[\s\._:-]*)");
    return regex_replace(name, pref, "");
}

long extraccionNumeroPrefijo(const string& s) {
    size_t i = 0;
    while (i < s.size() && isspace((unsigned char)s[i])) ++i;
    long num = 0;
    bool found = false;
    while (i < s.size() && isdigit((unsigned char)s[i])) {
        found = true;
        num = num * 10 + (s[i] - '0');
        ++i;
    }
    return found ? num : -1;
}

// Crea lotes en memoria a partir del vector files.
// Cada lote contiene hasta loteSize elementos.
static vector<vector<fs::directory_entry>> crearLotes(const vector<fs::directory_entry>& files, int loteSize) {
    if (loteSize <= 0) loteSize = 6;
    vector<vector<fs::directory_entry>> lotes;
    lotes.reserve((files.size() + loteSize - 1) / loteSize);
    for (size_t i = 0; i < files.size(); i += static_cast<size_t>(loteSize)) {
        size_t end = min(files.size(), i + static_cast<size_t>(loteSize));
        vector<fs::directory_entry> lote;
        lote.reserve(end - i);
        for (size_t j = i; j < end; ++j) lote.push_back(files[j]);
        lotes.push_back(move(lote));
    }
    return lotes;
}

// Crea el mapa y retorna también los lotes en memoria (si loteSize>0).
vector<vector<fs::directory_entry>> create_map(const string& dir, const string& out, int loteSize) {
    try {
        if (!fs::exists(dir) || !fs::is_directory(dir)) {
            cerr << "Directorio no encontrado: " << dir << "\n";
            return {};
        }

        vector<fs::directory_entry> files;
        for (auto &e : fs::directory_iterator(dir)) {
            if (e.is_regular_file()) files.push_back(e);
        }
        if (files.empty()) {
            cerr << "No se encontraron archivos en: " << dir << "\n";
            return {};
        }

        // ordenar respetando prefijo numérico si existe
        sort(files.begin(), files.end(), [](const fs::directory_entry& a, const fs::directory_entry& b){
            string na = a.path().filename().string();
            string nb = b.path().filename().string();
            long aa = extraccionNumeroPrefijo(na);
            long bb = extraccionNumeroPrefijo(nb);
            if (aa >= 0 && bb >= 0 && aa != bb) return aa < bb;
            return a.path().stem().string() < b.path().stem().string();
        });

        // escribir mapa
        fs::create_directories(fs::path(out).parent_path());
        ofstream os(out, ios::trunc);
        if (!os.is_open()) {
            cerr << "No se pudo crear archivo de salida: " << out << "\n";
            return {};
        }
        int id = 1;
        for (auto &e : files) {
            string stem = e.path().stem().string();
            string title = limpiarTitulo(stem);
            os << id << "; " << title << "\n";
            ++id;
        }
        os.close();
        cout << "Mapa creado: " << out << " (" << (id - 1) << " entradas)\n";

        // crear lotes en memoria y devolverlos
        auto lotes = crearLotes(files, loteSize);
        return lotes;

    } catch (const exception& ex) {
        cerr << "Error: " << ex.what() << "\n";
        return {};
    }
}