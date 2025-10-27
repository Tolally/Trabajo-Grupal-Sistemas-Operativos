#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>
#include <cctype>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <iomanip>
#include <unistd.h>
#include <algorithm>

using namespace std;
namespace fs = std::filesystem;

// Lee variable de entorno.
static int getEnvInt(const char* key, int def) {
    const char* v = std::getenv(key);
    if (!v || !*v) return def;
    try {
        return std::max(1, stoi(string(v)));
    } catch (...) {
        return def;
    }
}

// Normalizar palabras
static inline string normalizar(const string& w) {
    string clean;
    clean.reserve(w.size());
    for (unsigned char c : w) {
        if (isalnum(c)) clean.push_back(static_cast<char>(tolower(c)));
    }
    return clean;
}

// Escribe (o sobrescribe) el archivo de mapeo ID->Nombre de libro.
static bool escribirMapaLibros(const fs::path& path, const vector<fs::path>& archivos) {
    ofstream out(path);
    if (!out.is_open()) return false;
    for (size_t i = 0; i < archivos.size(); ++i) {
        out << (i + 1) << "; " << archivos[i].filename().string() << "\n";
    }
    return true;
}

// Convierte un time_point a string.
static inline long long to_epoch(const chrono::system_clock::time_point& tp) {
    return chrono::duration_cast<chrono::seconds>(tp.time_since_epoch()).count();
}

int main(int argc, char* argv[]) {
    cout << "==============================\n";
    cout << " PID del proceso actual: " << getpid() << endl;
    cout << "==============================\n";
    if (argc != 3) {
        cerr << "Uso: create_index_parallel <archivo_salida.idx> <carpeta_libros>\n";
        return 1;
    }

    string nombreArchivo = argv[1];
    string carpetaLibros = argv[2];

    if (nombreArchivo.size() < 4 || nombreArchivo.substr(nombreArchivo.size() - 4) != ".idx") {
        cerr << "Error: el archivo de índice debe tener extensión .idx\n";
        return 1;
    }

    // Carpeta de salida para índices y demás archivos.
    fs::path carpetaIndices = "data/indices";
    if (!fs::exists(carpetaIndices)) {
        fs::create_directories(carpetaIndices);
    }

    fs::path rutaSalida = carpetaIndices / nombreArchivo;

    // Sobreescribir
    if (fs::exists(rutaSalida)) {
        cout << "El archivo \"" << rutaSalida.string() << "\" ya existe. Desea sobrescribir? (s/n): ";
        string resp; cin >> resp; cin.ignore(numeric_limits<streamsize>::max(), '\n');
        while (resp != "s" && resp != "S" && resp != "n" && resp != "N") {
            cout << "Respuesta inválida. Ingrese 's' para sí o 'n' para no: ";
            cin >> resp; cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        if (resp == "n" || resp == "N") {
            cout << "Operación cancelada. No se creó el índice.\n";
            return 0;
        }
    }

    // Cargar configuración
    int nThreads = getEnvInt("N_THREADS", std::max(1u, std::thread::hardware_concurrency()));
    int nLote    = getEnvInt("N_LOTE", 4);
    const char* mapaEnv = std::getenv("MAPA_LIBROS");
    const char* logEnv  = std::getenv("PARALLEL_LOG");
    fs::path rutaMapa = mapaEnv && *mapaEnv ? fs::path(mapaEnv) : (carpetaIndices / "mapa_libros.txt");
    fs::path rutaLog  = logEnv  && *logEnv  ? fs::path(logEnv)  : (carpetaIndices / "parallel.log");

    // Recolectar archivos .txt
    if (!fs::exists(carpetaLibros) || !fs::is_directory(carpetaLibros)) {
        cerr << "La carpeta \"" << carpetaLibros << "\" no existe o no es un directorio.\n";
        return 1;
    }
    vector<fs::path> txtFiles;
    for (const auto& entry : fs::directory_iterator(carpetaLibros)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") txtFiles.push_back(entry.path());
    }
    if (txtFiles.empty()) {
        cerr << "No se encontraron archivos .txt en \"" << carpetaLibros << "\". No se creó el índice.\n";
        return 1;
    }

    // Ordenar
    sort(txtFiles.begin(), txtFiles.end());

    // Crear mapa de libros
    if (!escribirMapaLibros(rutaMapa, txtFiles)) {
        cerr << "No se pudo crear MAPA_LIBROS en: " << rutaMapa << "\n";
        return 1;
    }

    // Estructuras globales compartidas
    map<string, map<int, int>> indice; // palabra -> (id_libro -> conteo)
    std::mutex mtxIndice;

    ofstream flog(rutaLog, ios::app);
    std::mutex mtxLog;
    if (!flog.is_open()) {
        cerr << "Advertencia: no se pudo abrir log para escritura: " << rutaLog << "\n";
    }

    auto log_evento = [&](const std::thread::id& tid, int idLibro, long palabras,
                          const chrono::system_clock::time_point& t0,
                          const chrono::system_clock::time_point& t1) {
        if (!flog.is_open()) return;
        static std::hash<std::thread::id> hasher;
        long long inicio = to_epoch(t0);
        long long termino = to_epoch(t1);
        size_t tidNum = hasher(tid);
        lock_guard<mutex> lk(mtxLog);
        flog << tidNum << ';' << idLibro << ';' << palabras << ';' << inicio << ';' << termino << "\n";
    };

    // Procesamiento por lotes
    const size_t total = txtFiles.size();
    for (size_t base = 0; base < total; base += static_cast<size_t>(nLote)) {
        size_t end = min(total, base + static_cast<size_t>(nLote));
        size_t batchSize = end - base;
        if (batchSize == 0) break;

        atomic<size_t> cursor{0};
        size_t workers = static_cast<size_t>(min<int>(nThreads, static_cast<int>(batchSize)));
        vector<thread> pool;
        pool.reserve(workers);

        auto worker = [&]() {
            size_t localIdx;
            while ((localIdx = cursor.fetch_add(1)) < batchSize) {
                size_t globalIdx = base + localIdx;
                const fs::path& path = txtFiles[globalIdx];
                int idLibro = static_cast<int>(globalIdx + 1);

                auto t0 = chrono::system_clock::now();
                ifstream f(path);
                if (!f.is_open()) {
                    // registrar intento fallido con 0 palabras
                    log_evento(this_thread::get_id(), idLibro, 0, t0, chrono::system_clock::now());
                    continue;
                }

                map<string, int> frecLocal;
                long totalPalabras = 0;
                string token;
                while (f >> token) {
                    string w = normalizar(token);
                    if (!w.empty()) { ++frecLocal[w]; ++totalPalabras; }
                }
                f.close();
                auto t1 = chrono::system_clock::now();

                // fusionar al índice global
                {
                    lock_guard<mutex> lk(mtxIndice);
                    for (const auto& kv : frecLocal) {
                        indice[kv.first][idLibro] += kv.second;
                    }
                }

                // registrar log
                log_evento(this_thread::get_id(), idLibro, totalPalabras, t0, t1);
            }
        };

        for (size_t i = 0; i < workers; ++i) pool.emplace_back(worker);
        for (auto& th : pool) th.join();
    }

    // Escribir índice
    ofstream out(rutaSalida);
    if (!out.is_open()) {
        cerr << "No se pudo crear el archivo de índice: " << rutaSalida << "\n";
        return 1;
    }
    for (const auto& [pal, docs] : indice) {
        out << pal;
        for (const auto& [docId, cnt] : docs) {
            out << ";(" << docId << "," << cnt << ")";
        }
        out << "\n";
    }
    out.close();

    fs::path salidaAbs = fs::absolute(rutaSalida);
    cout << "Índice invertido paralelo creado correctamente en " << salidaAbs << "\n";
    cout << "MAPA_LIBROS: " << fs::absolute(rutaMapa) << "\n";
    if (flog.is_open()) cout << "LOG paralelo: " << fs::absolute(rutaLog) << "\n";
    return 0;
}
