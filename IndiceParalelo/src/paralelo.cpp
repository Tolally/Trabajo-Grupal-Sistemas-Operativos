#include "../include/paralelo.h"
#include "../include/create_map.h"
#include "../include/create_index_paralel.h"


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

    if (maxThreadsH <= static_cast<unsigned int>(nThreadsEnv))
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
        if (value > 0 && value <= max)  
            return value;

        cout << "Debe ingresar un entero mayor que 0 y menor que " << max << ". Intente nuevamente.\n";
    }
}
/*
// procesa un lote: construye texto de log, crea y guarda índice parcial,
// devuelve la duración en microsegundos en 'dur_us' y acumula el texto en 'out'.
long long procesarLote(const vector<fs::directory_entry>& lote,
                       int loteId,
                       int threadId,
                       const fs::path& carpetaIndices,
                       string &out)
{
    using clk = chrono::high_resolution_clock;
    auto t0 = clk::now();

    out += "Thread " + to_string(threadId) + " procesando lote " + to_string(loteId)
         + " (" + to_string(lote.size()) + " libros)\n";

    for (size_t i = 0; i < lote.size(); ++i) {
        out += "  [" + to_string(loteId) + "." + to_string(i + 1) + "] "
             + lote[i].path().filename().string() + "\n";
    }

    // Nombre único por thread/lote para el índice parcial
    string nombreArchivo = "partial_" + to_string(threadId) + "_" + to_string(loteId) + ".idx";
    fs::path rutaSalida = carpetaIndices / nombreArchivo;

    // crear índice parcial y guardarlo
    Index parcial = crearIndiceInvertidoPorLote(rutaSalida.string(), lote);
    if (!guardarIndice(rutaSalida.string(), parcial)) {
        out += "Error: no se pudo guardar índice parcial: " + rutaSalida.string() + "\n";
    }

    long long dur_us = chrono::duration_cast<chrono::microseconds>(clk::now() - t0).count();

    out += "Thread " + to_string(threadId) + " finalizó lote " + to_string(loteId)
         + " (tiempo " + to_string(dur_us) + " µs)\n";

    return dur_us;
}

// funcion que procesa un lote (sustituir por la lógica real de creación de índice)
void procesarLote(const vector<fs::directory_entry>& lote, int loteId, int threadId) {
    cout << "Thread " << threadId << " procesando lote " << loteId << " (" << lote.size() << " libros)\n";

    fs::path carpetaIndices = "data/indices";
    if (!fs::exists(carpetaIndices)) {
        fs::create_directories(carpetaIndices);
    }

    const char* nombreArchivo = "textX.idx";

    fs::path rutaSalida = carpetaIndices / nombreArchivo;

    crearIndiceInvertidoPorLote(rutaSalida.string(), lote);

    cout << "Thread " << threadId << " finalizó lote " << loteId << "\n";
}*/

// función auxiliar para mergear un índice parcial en el índice destino
void mergeIndex (Index &dest, const Index &src){
    for (const auto &kv : src) {
        const string &pal = kv.first;
        const auto &docs = kv.second;
        auto &destDocs = dest[pal]; // crea si hace falta
        for (const auto &dd : docs) {
            destDocs[dd.first] += dd.second;
        }
    }
};

void invertidoParalelo(){
    cout << "==============================\n";
    cout << " PID del proceso actual: " << getpid() << "\n";
    cout << "==============================\n\n";

    unsigned int nThreadsMax = obtenerThreadsHardware();
    const char* envThreads = getenv("N_THREADS");
    const char* envLote = getenv("N_LOTE");
    int nThreadsEnv = envThreads ? atoi(envThreads) : 1;
    int nLote = envLote ? atoi(envLote) : 6;
    if (nLote <= 0) nLote = 6;

    cout << "==============================\n";
    cout << "  Indice Invertido Paralelo\n\n";
    cout << " Threads hardware detectados: " << nThreadsMax << "\n";
    cout << " Threads .env detectados: " << nThreadsEnv << "\n";
    cout << "==============================\n\n";

    string salida;

    // generar mapa y lotes
    vector<vector<fs::directory_entry>> lotes = create_map("Menu/libros", "data/indices/mapa.txt", nLote);
    if (lotes.empty()) {
        cout << "No hay lotes para procesar.\n";

        cout << "Presione Enter para volver...\n";
        // descartar resto de la línea previa (si lo hay) antes de usar getline
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        getline(cin, salida);
        return;
    }

    int useThreads = pedirNumeroThreads(nThreadsMax, nThreadsEnv);
    if (useThreads <= 0) {
        cout << "Operación cancelada o número de threads inválido.\n";

        cout << "Presione Enter para volver...\n";
        // descartar resto de la línea previa (si lo hay) antes de usar getline
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        getline(cin, salida);
        return;
    }

    // no crear más threads que lotes
    if (static_cast<unsigned int>(useThreads) > lotes.size())
        useThreads = static_cast<int>(lotes.size());

    cout << "Usando " << useThreads << " thread(s) para la operación.\n";

    // asignación dinámica de lotes: cada thread toma el siguiente índice atómico
    atomic<size_t> nextLote{0};
    size_t totalLotes = lotes.size();

    // Path carpeta indices
    fs::path carpetaIndices = "data/indices";
    if (!fs::exists(carpetaIndices)) {
        fs::create_directories(carpetaIndices);
    }

    string nombreArchivo;

    while (true) {
        cout << "Ingrese nombre del archivo a crear (debe terminar en .idx, o '0' para cancelar): ";
        cin >> nombreArchivo;
        if (nombreArchivo == "0") { 
            cout << "Operación cancelada por el usuario.\n"; 
            cout << "Presione Enter para volver...\n";
            // descartar resto de la línea previa (si lo hay) antes de usar getline
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            getline(cin, salida);
            return; 
        }
        if (nombreArchivo.size() >= 4 && nombreArchivo.substr(nombreArchivo.size() - 4) == ".idx") 
            break;

        cerr << "Error: el archivo debe tener extensión .idx\n";
    }

    fs::path rutaSalida = carpetaIndices / nombreArchivo;

    Index indice;

    // Buffers por thread (cada hilo escribe solo en su índice)
    vector<string> threadOutputs(useThreads);          // acumulador de salida por hilo
    vector<int> countPerThread(useThreads, 0);         // contadores
    vector<long long> timePerThread(useThreads, 0);    // tiempos acumulados (us)

    // vector para almacenar índices parciales producidos por cada hilo (sin race: hilo t escribe solo en slot t)
    vector<Index> partialIndices(useThreads);


    // start latch: todos los hilos esperan hasta que el master levante la bandera
    atomic<bool> start{false};

    vector<thread> threadPool;
    threadPool.reserve(useThreads);

    for (int t = 0; t < useThreads; ++t) {
        threadPool.emplace_back([t, &nextLote, totalLotes, &lotes,
                                 &threadOutputs, &countPerThread, &timePerThread, &start,
                                 carpetaIndices, &partialIndices]() {
            using clk = chrono::high_resolution_clock;
            string out; // buffer local por hilo

            // esperar señal de inicio
            while (!start.load(memory_order_acquire)) this_thread::yield();

            // Procesar lotes dinámicamente
            while (true) {
                size_t idx = nextLote.fetch_add(1, memory_order_relaxed);
                if (idx >= totalLotes) break;

                // Crear índice parcial del lote (cada hilo obtiene su Index)
                Index parcial = crearIndiceInvertidoPorLote(lotes[idx]);

                // mergear parcial en el slot del hilo (solo este hilo escribe partialIndices[t])
                mergeIndex(partialIndices[t], parcial);

                // registrar salida y tiempo
                auto t0 = clk::now();

                out += "Thread " + to_string(t) + " procesando lote " + to_string(idx + 1)
                     + " (" + to_string(lotes[idx].size()) + " libros)\n";
                for (size_t i = 0; i < lotes[idx].size(); ++i) {
                    out += "  [" + to_string(idx + 1) + "." + to_string(i + 1) + "] "
                         + lotes[idx][i].path().filename().string() + "\n";
                }
                auto dur_us = chrono::duration_cast<chrono::microseconds>(clk::now() - t0).count();

                ++countPerThread[t];
                timePerThread[t] += dur_us;

                out += "Thread " + to_string(t) + " finalizó lote " + to_string(idx + 1)
                     + " (tiempo " + to_string(dur_us) + " µs)\n";
            }

            // almacenar el output acumulado en el slot reservado (escrito por único hilo)
            threadOutputs[t].swap(out);
        });
    }

    // lanzar a la vez todos los hilos
    start.store(true, memory_order_release);

    for (auto &th : threadPool) if (th.joinable()) th.join();

    // Después de join: mergear todos los índices parciales en 'indice' (hilo principal)
    for (int t = 0; t < useThreads; ++t) {
        mergeIndex(indice, partialIndices[t]);
    }

    // guardar índice final en rutaSalida
    if (!guardarIndice(rutaSalida.string(), indice)) {
        cerr << "Error: no se pudo guardar el índice final en " << rutaSalida << "\n";
    }

    // resumen (convertir µs a ms si quieres)
    cout << "Resumen por thread:\n";
    for (int t = 0; t < useThreads; ++t) {
        double ms = timePerThread[t] / 1000.0;
        cout << " Thread " << t << ": " << countPerThread[t]
             << " lotes, tiempo total " << ms << " ms (" << timePerThread[t] << " µs)\n";
    }


    // Opción para ver el log o no en pantalla
    string verLog;

    while (true) {
        cout << "Ingrese '1' si desea ver el LOG en pantalla ('0' para cancelar): ";
        cin >> verLog;
        if (verLog == "0") { 
            cout << "Operación cancelada por el usuario.\n"; 
            break; 
        }
        if (verLog == "1") {
            // impresión secuencial en el hilo principal (sin concurrencia)
            for (int t = 0; t < useThreads; ++t) {
                cout << threadOutputs[t];
            }
            break;
        }

        cerr << "Error: debe ingresar '1' o '0'\n";
    }

    // espera para salir (comportamiento previo)
    cout << "Presione Enter para volver...\n";
    // descartar resto de la línea previa (si lo hay) antes de usar getline
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    getline(cin, salida);

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