#include "../include/create_index_paralel.h"

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

// Procesa un lote de archivos y devuelve el índice parcial en memoria.
// Entrada: vector de directory_entry (cada entry debe ser archivo regular)
Index crearIndiceInvertidoPorLote(const vector<fs::directory_entry>& lote) {
    Index indice;

    for (const auto& entry : lote) {
        if (!entry.is_regular_file()) continue;
        ifstream f(entry.path());
        if (!f.is_open()) continue;
        string palabra;
        string filename = entry.path().filename().string();
        while (f >> palabra) {
            string clean = normalizar(palabra);
            if (!clean.empty()) {
                indice[clean][filename]++;
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

// Función pública: crear índice a partir de una carpeta completa (usa las funciones anteriores)
bool crearIndiceInvertido(const string& nombreArchivo, const string& carpetaLibros) {
    // Verificar extensión .idx
    if (nombreArchivo.size() < 4 || nombreArchivo.substr(nombreArchivo.size() - 4) != ".idx") {
        cerr << "Error: el archivo de índice debe tener extensión .idx\n";
        return false;
    }

    // Revisar existencia de carpeta
    if (!fs::exists(carpetaLibros) || !fs::is_directory(carpetaLibros)) {
        cerr << "La carpeta \"" << carpetaLibros << "\" no existe o no es un directorio.\n";
        return false;
    }

    // Reunir archivos .txt en un vector<directory_entry>
    vector<fs::directory_entry> txtFiles;
    for (const auto& entry : fs::directory_iterator(carpetaLibros)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") {
            txtFiles.push_back(entry);
        }
    }
    if (txtFiles.empty()) {
        cerr << "No se encontraron archivos .txt en \"" << carpetaLibros << "\". No se creó el índice.\n";
        return false;
    }

    // Crear índice usando la función por lote (aquí todo el conjunto es un único "lote")
    Index indice = crearIndiceInvertidoPorLote(txtFiles);

    return true;
}

/*int main(int argc, char* argv[]) {
    cout << "==============================\n";
    cout << " PID del proceso actual: " << getpid() << endl;
    cout << "==============================\n";
    if (argc != 3) {
        std::cerr << "Uso: create_index <archivo_salida.idx> <carpeta_libros>\n";
        return 1;
    }
    std::string nombreArchivo = argv[1];
    std::string carpeta = argv[2];

    fs::path carpetaIndices = "data/indices";
    if (!fs::exists(carpetaIndices)) {
        fs::create_directories(carpetaIndices);
    }

    fs::path rutaSalida = carpetaIndices / nombreArchivo;

    if (!crearIndiceInvertido(rutaSalida.string(), carpeta)) {
        std::cerr << "Error al crear índice invertido.\n";
        return 1;
    }

    fs::path salidaAbs = fs::absolute(rutaSalida);
    std::cout << "Índice invertido creado correctamente en " << salidaAbs << "\n";
    return 0;
}*/