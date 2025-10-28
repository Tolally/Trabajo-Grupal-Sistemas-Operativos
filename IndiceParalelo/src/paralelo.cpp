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
    vector<long long> timePerThread(useThreads, 0);    // tiempos acumulados (ms)

    // vector para almacenar índices parciales producidos por cada hilo (sin race: hilo t escribe solo en slot t)
    vector<Index> partialIndices(useThreads);

    // vector para almacenar logs por hilo (cada hilo escribe sólo en partialLogs[t])
    vector<vector<LogEntry>> partialLogs(useThreads);

    // start latch: todos los hilos esperan hasta que el master levante la bandera
    atomic<bool> start{false};

    vector<thread> threadPool;
    threadPool.reserve(useThreads);

    // ... antes de crear los hilos, cargar el mapa:
    auto mapaLibros = cargarMapaLibros("data/indices/mapa.txt");

    for (int t = 0; t < useThreads; ++t) {
        threadPool.emplace_back([t, &nextLote, totalLotes, &lotes,
                                 &threadOutputs, &countPerThread, &timePerThread, &start,
                                 carpetaIndices, &partialIndices, &partialLogs, mapaLibros]() {
            using clk = chrono::high_resolution_clock;
            string out; // buffer local por hilo

            // esperar señal de inicio
            while (!start.load(memory_order_acquire)) this_thread::yield();

            // Procesar lotes dinámicamente
            while (true) {
                size_t idx = nextLote.fetch_add(1, memory_order_relaxed);
                if (idx >= totalLotes) break;

                out += "Thread " + to_string(t) + " procesando lote " + to_string(idx + 1)
                     + " (" + to_string(lotes[idx].size()) + " libros)\n";

                // procesar archivo por archivo (para obtener timestamps por libro)
                for (size_t i = 0; i < lotes[idx].size(); ++i) {
                    const auto &entry = lotes[idx][i];

                    // determinar docId consistente con crearIndiceInvertidoPorLote(...)
                    string filename = entry.path().filename().string();
                    string stem = entry.path().stem().string();
                    string titleKey = limpiarTitulo(stem);
                    string docId = filename; // fallback
                    auto itMap = mapaLibros.find(titleKey);
                    if (itMap != mapaLibros.end()) docId = itMap->second;

                    // medir inicio
                    auto t0 = clk::now();

                    // crear índice parcial del único fichero (usa la versión que recibe el mapa)
                    vector<fs::directory_entry> single{ entry };
                    Index fileIdx = crearIndiceInvertidoPorLote(single, mapaLibros);

                    // contar palabras para el docId (sumar las frecuencias en fileIdx)
                    size_t wordCount = 0;
                    for (const auto &kv : fileIdx) {
                        const auto &docs = kv.second;
                        auto itDoc = docs.find(docId);
                        if (itDoc != docs.end()) wordCount += static_cast<size_t>(itDoc->second);
                    }

                    // mergear fileIdx en el índice parcial del hilo
                    mergeIndex(partialIndices[t], fileIdx);

                    // medir fin
                    auto t1 = clk::now();

                    // calcular milisegundos
                    long long start_ms = chrono::duration_cast<chrono::milliseconds>(t0.time_since_epoch()).count();
                    long long end_ms   = chrono::duration_cast<chrono::milliseconds>(t1.time_since_epoch()).count();
                    long long dur_ms   = chrono::duration_cast<chrono::milliseconds>(t1 - t0).count();

                    // actualizar estadísticas del hilo (ahora en ms)
                    timePerThread[t] += dur_ms;

                    // registrar log (cada hilo escribe sólo en partialLogs[t])
                    partialLogs[t].push_back(LogEntry{t, docId, wordCount, start_ms, end_ms});

                    // construir salida legible (opcional)
                    out += "  [" + to_string(idx + 1) + "." + to_string(i + 1) + "] " + filename + "\n";
                }

                ++countPerThread[t];

                out += "Thread " + to_string(t) + " finalizó lote " + to_string(idx + 1) + "\n";
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

    // Escribir log de ejecución (csv)
    fs::path rutaLog = carpetaIndices / (nombreArchivo + ".runlog.csv");
    ofstream logf(rutaLog);
    if (logf.is_open()) {
        logf << "thread,book,word_count,start_us,end_us\n";
        for (int t = 0; t < useThreads; ++t) {
            for (const auto &e : partialLogs[t]) {
                logf << e.threadId << ',' 
                     << '"' << e.bookId << '"' << ','
                     << e.wordCount << ',' << e.start_ms << ',' << e.end_ms << '\n';
            }
        }
        logf.close();
    } else {
        cerr << "No se pudo abrir archivo de log: " << rutaLog << "\n";
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