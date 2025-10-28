#include "../include/paralelo.h"
#include "../include/create_map.h"
#include "../include/create_index2.h"


static unsigned int obtenerThreadsHardware() {
    unsigned int hc = thread::hardware_concurrency();
    if (hc == 0) { // fallback en Linux
        long n = sysconf(_SC_NPROCESSORS_ONLN);
        if (n > 0) 
            hc = static_cast<unsigned int>(n);
    }
    return (hc == 0) ? 1u : hc; // al menos 1
}

// Valida y pide al usuario un número de threads: devuelve 0 si se cancela.
int pedirNumeroThreads(unsigned int maxThreadsH, int nThreadsEnv) {
    if (maxThreadsH <= 1) {
        cout << "No hay más de 1 thread disponible en este equipo. Operación cancelada.\n";
        return 0;
    }

    int max = 0;

    if (maxThreadsH >= static_cast<unsigned int>(nThreadsEnv))
        max = maxThreadsH;
    else 
        max = nThreadsEnv;

    int value = -1;

    while (true) {
        cout << "Ingrese la cantidad de threads a usar (1 - " << (max) << ", 0 para cancelar): ";
        if (!(cin >> value)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Entrada inválida. Debe ingresar un número entero.\n";
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // limpiar resto de la línea

        if (value == 0) 
            return 0; // cancelar
        if (value > 0 && value < max)  
            return value;

        cout << "Debe ingresar un entero mayor que 0 y menor que " << max << ". Intente nuevamente.\n";
    }
}

// funcion que procesa un lote (sustituir por la lógica real de creación de índice)
static void procesarLote(const vector<fs::directory_entry>& lote, int loteId, int threadId) {
    cout << "Thread " << threadId << " procesando lote " << loteId << " (" << lote.size() << " libros)\n";
    for (size_t i = 0; i < lote.size(); ++i) {
        const auto &entry = lote[i];
        cout << "  [" << loteId << "." << (i + 1) << "] " << entry.path().filename().string() << "\n";
        // Aquí va la lógica real, por ejemplo:
        // crearIndiceInvertido(...); o abrir archivo y procesar
        // crearIndiceInvertido(/*nombreArchivo*/, /*carpetaLibros*/);
    }
    cout << "Thread " << threadId << " finalizó lote " << loteId << "\n";
}


void invertidoParalelo(){
    cout << "==============================\n";
    cout << " PID del proceso actual: " << getpid() << "\n";
    cout << "==============================\n\n";

    unsigned int nThreadsMax = obtenerThreadsHardware();
    const char* envThreads = std::getenv("N_THREADS");
    const char* envLote = std::getenv("N_LOTE");
    int nThreadsEnv = envThreads ? atoi(envThreads) : 1;
    int nLote = envLote ? atoi(envLote) : 6;
    if (nLote <= 0) nLote = 6;

    cout << "==============================\n";
    cout << "  Indice Invertido Paralelo\n\n";
    cout << "Threads hardware detectados: " << nThreadsMax << "\n";
    cout << "Threads .env detectados: " << nThreadsEnv << "\n";
    cout << "==============================\n\n";

    // generar mapa y lotes
    static vector<vector<fs::directory_entry>> lotes = create_map("Menu/libros", "data/indices/mapa.txt", nLote);
    if (lotes.empty()) {
        cout << "No hay lotes para procesar.\n";
        return;
    }

    int useThreads = pedirNumeroThreads(nThreadsMax, nThreadsEnv);
    if (useThreads <= 0) {
        cout << "Operación cancelada o número de threads inválido.\n";
        return;
    }

    // no crear más threads que lotes
    if (static_cast<unsigned int>(useThreads) > lotes.size())
        useThreads = static_cast<int>(lotes.size());

    cout << "Usando " << useThreads << " thread(s) para la operación.\n";

    // asignación dinámica de lotes: cada thread toma el siguiente índice atómico
    atomic<size_t> nextLote{0};
    size_t totalLotes = lotes.size();

    vector<thread> threadPool;
    threadPool.reserve(useThreads);

    for (int t = 0; t < useThreads; ++t) {
        threadPool.emplace_back([t, &nextLote, totalLotes]() {
            while (true) {
                size_t idx = nextLote.fetch_add(1, std::memory_order_relaxed);
                if (idx >= totalLotes) break; // no quedan lotes
                // Procesar lote idx
                procesarLote(lotes[idx], static_cast<int>(idx + 1), t);
            }
            cout << "Thread " << t << " terminado (no quedan lotes).\n";
        });
    }

    for (auto &th : threadPool) if (th.joinable()) th.join();

    cout << "Todos los lotes procesados.\n";

    // espera para salir (comportamiento previo)
    string salida;
    cout << "Presione Enter para volver...\n";
    std::getline(cin, salida);
    return;
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    try {
        // Llama la función principal del módulo paralelo
        invertidoParalelo();
        return 0;
    } catch (const exception& ex) {
        cerr << "Error: " << ex.what() << endl;
        return 1;
    } catch (...) {
        cerr << "Error desconocido\n";
        return 1;
    }
}