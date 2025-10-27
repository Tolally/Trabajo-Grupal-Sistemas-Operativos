#include "../include/paralelo.h"

static unsigned int obtenerThreadsHardware() {
    unsigned int hc = std::thread::hardware_concurrency();
    if (hc == 0) { // fallback en Linux
        long n = sysconf(_SC_NPROCESSORS_ONLN);
        if (n > 0) 
            hc = static_cast<unsigned int>(n);
    }
    return (hc == 0) ? 1u : hc; // al menos 1
}

// Valida y pide al usuario un número de threads: devuelve 0 si se cancela.
int pedirNumeroThreads(unsigned int maxThreads) {
    if (maxThreads <= 1) {
        cout << "No hay más de 1 thread disponible en este equipo. Operación cancelada.\n";
        return 0;
    }

    int value = -1;
    while (true) {
        cout << "Ingrese la cantidad de threads a usar (1 - " << (maxThreads) << ", 0 para cancelar): ";
        if (!(cin >> value)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Entrada inválida. Debe ingresar un número entero.\n";
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // limpiar resto de la línea

        if (value == 0) 
            return 0; // cancelar
        if (value > 0 && static_cast<unsigned int>(value) < maxThreads) 
            return value;

        cout << "Debe ingresar un entero mayor que 0 y menor que " << maxThreads << ". Intente nuevamente.\n";
    }
}

void invertidoParalelo(){
    cout << "==============================\n";
    cout << " PID del proceso actual: " << getpid() << endl;
    cout << "==============================\n\n";

    unsigned int nthreads = obtenerThreadsHardware();
    cout << "==============================\n";
    cout << "  Indice Invertido Paralelo\n\n";
    cout << "Threads hardware detectados: " << nthreads << "\n";
    cout << "==============================\n\n";

    int useThreads = pedirNumeroThreads(nthreads);

    cout << "Usando " << useThreads << " thread(s) para la operación.\n";

    // preparar y lanzar threads (ejemplo)
    vector<thread> threadPool;
    threadPool.reserve(useThreads);

    for (int i = 0; i < useThreads; ++i) {
        threadPool.emplace_back([i]() {
            cout << "Thread " << i << " iniciado. id=" << this_thread::get_id() << "\n";
            // ... tarea del thread ...
        });
    }
    for (auto &t : threadPool) if (t.joinable()) t.join();

    return;

}

int main(int argc, char** argv) {
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