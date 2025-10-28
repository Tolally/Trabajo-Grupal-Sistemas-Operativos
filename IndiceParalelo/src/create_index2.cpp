#include "../include/create_index2.h"

namespace fs = std::filesystem;

bool crearIndiceInvertido(const string& nombreArchivo, const string& carpetaLibros) {
    // Verificar extensión
    if (nombreArchivo.size() < 4 || nombreArchivo.substr(nombreArchivo.size() - 4) != ".idx") {
        cerr << "Error: el archivo de índice debe tener extensión .idx\n";
        return false;
    }

    // Revisar si el archivo ya existe
    if (fs::exists(nombreArchivo)) {
        cout << "El archivo \"" << nombreArchivo << "\" ya existe. Desea sobrescribir? (s/n): ";
        string resp;
        cin >> resp;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        while (true) {
            if (resp == "s" || resp == "S" || resp == "n" || resp == "N") break;
            cout << "Respuesta inválida. Ingrese 's' para sí o 'n' para no: ";
            cin >> resp;
        }
        if (resp == "n" || resp == "N") {
            cout << "Operación cancelada. No se creó el índice.\n";
            return false;
        }

    }

    // Revisar si existen archivos .txt en la carpeta
    vector<fs::path> txtFiles;
    if (!fs::exists(carpetaLibros) || !fs::is_directory(carpetaLibros)) {
        cerr << "La carpeta \"" << carpetaLibros << "\" no existe o no es un directorio.\n";
        return false;
    }
    for (const auto& entry : fs::directory_iterator(carpetaLibros)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") {
            txtFiles.push_back(entry.path());
        }
    }

    if (txtFiles.empty()) {
        cerr << "No se encontraron archivos .txt en \"" << carpetaLibros << "\". No se creó el índice.\n";
        return false;
    }

    // Crear índice invertido
    map<string, map<string, int>> indice;
    for (const auto& path : txtFiles) {
        ifstream f(path);
        if (!f.is_open()) continue;
        string palabra;
        while (f >> palabra) {
            // Normalizar palabra: quitar puntuación, convertir a minúscula
            string clean;
            for (char c : palabra) {
                if (isalnum(static_cast<unsigned char>(c))) clean.push_back(tolower(c));
            }
            if (!clean.empty()) {
                indice[clean][path.filename().string()]++;
            }
        }
    }

    // Guardar índice en archivo
    ofstream out(nombreArchivo);
    if (!out.is_open()) {
        cerr << "No se pudo crear el archivo de índice: " << nombreArchivo << "\n";
        return false;
    }
    for (const auto& [pal, docs] : indice) {
        out << pal;
        for (const auto& [doc, cnt] : docs) {
            out << ";(" << doc << "," << cnt << ")";
        }
        out << "\n";
    }
    out.close();
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