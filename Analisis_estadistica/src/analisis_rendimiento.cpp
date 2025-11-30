#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>
#include <chrono>
#include <fstream>
#include <unistd.h>
#include <algorithm>
#include <thread>

using namespace std;

// asegurar que se pueda leer el env
static void cargarDotEnvSiExiste() {
    const char* already = getenv("__DOTENV_LOADED");
    if (already && string(already) == "1") return;
    const char* candidatas[] = { ".env", "../.env", "../../.env" };
    for (const char* p : candidatas) {
        ifstream f(p);
        if (!f.is_open()) continue;
        string linea;
        while (getline(f, linea)) {
            if (linea.empty() || linea[0] == '#') continue;
            auto pos = linea.find('=');
            if (pos == string::npos) continue;
            string k = linea.substr(0, pos);
            string v = linea.substr(pos + 1);
            
            // trim
            while (!k.empty() && isspace(static_cast<unsigned char>(k.front()))) k.erase(0, 1);
            while (!k.empty() && isspace(static_cast<unsigned char>(k.back()))) k.pop_back();
            while (!v.empty() && isspace(static_cast<unsigned char>(v.front()))) v.erase(0, 1);
            while (!v.empty() && isspace(static_cast<unsigned char>(v.back()))) v.pop_back();

            if (k.empty()) continue;
            if (!getenv(k.c_str())) {
                setenv(k.c_str(), v.c_str(), 0);
            }
        }
        setenv("__DOTENV_LOADED", "1", 1);
        break;
    }
}

static string getEnvOr(const char* key, const string& fallback) {
    const char* v = getenv(key);
    if (v && *v) return string(v);
    return fallback;
}

int main() {
    // Cargar variables de entorno desde .env
    cargarDotEnvSiExiste();

    string pythonScript = getEnvOr("PYTHON_SCRIPT_PATH", "");
    string indiceProgram = getEnvOr("INDICE_INVET_PARALELO", "");
    string sMaxLen = getEnvOr("MAX_THREADS_ARRAY_LENGTH", "");
    int maxLen = atoi(sMaxLen.c_str());
    string logPath = getEnvOr("ANALISIS_LOG_PATH", "");
    string idxTemp = getEnvOr("ANALISIS_IDX_TEMP", "");

    if (pythonScript.empty() || indiceProgram.empty() || logPath.empty() || idxTemp.empty()) {
        cerr << "Error: Faltan variables de entorno necesarias en .env\n";
        cout << "Presione Enter para volver.";
        cin.get();
        return 1;
    }

    cout << "=== Análisis de Rendimiento ===\n";
    unsigned int maxThreads = std::thread::hardware_concurrency();
    if (maxThreads == 0) maxThreads = 1;
    cout << "Threads disponibles en el sistema: " << maxThreads << "\n";
    cout << "Ingrese la cantidad de threads a probar separados por espacio (ej: 1 2 4 8): ";
    string line;
    getline(cin, line);
    stringstream ss(line);
    string token;
    vector<int> threads;
    bool error = false;

    while (ss >> token) {
        // Validar que el token sea un valor entero positivo
        if (token.empty()) continue;
        bool isNum = true;
        for (char c : token) {
            if (!isdigit(c)) {
                isNum = false;
                break;
            }
        }
        
        if (!isNum) {
            cout << "Error: Valor inválido detectado '" << token << "'. Solo se permiten números enteros.\n";
            error = true;
            break;
        }

        try {
            int val = stoi(token);
            if (val <= 0) {
                cout << "Error: El número de threads debe ser mayor a 0 ('" << token << "').\n";
                error = true;
                break;
            }
            if (val > (int)maxThreads) {
                cout << "Error: El número de threads (" << val << ") supera el máximo del sistema (" << maxThreads << ").\n";
                error = true;
                break;
            }
            threads.push_back(val);
        } catch (...) {
            cout << "Error: Valor inválido '" << token << "'.\n";
            error = true;
            break;
        }
    }

    if (error) {
        cout << "Operación cancelada debido a errores en la entrada.\n";
        cout << "Presione Enter para volver.";
        cin.get();
        return 1;
    }

    if (threads.empty()) {
        cout << "No se ingresaron valores válidos.\n";
        cout << "Presione Enter para volver.";
        cin.get();
        return 1;
    }

    if (threads.size() > (size_t)maxLen) {
        cout << "Demasiados valores. Máximo permitido: " << maxLen << "\n";
        cout << "Presione Enter para volver.";
        cin.get();
        return 1;
    }

    // para estar seguros
    ofstream logFile(logPath);
    if (!logFile.is_open()) {
        cerr << "No se pudo crear el log: " << logPath << "\n";
        cout << "Presione Enter para volver.";
        cin.get();
        return 1;
    }
    logFile << "threads,time_ms\n";

    for (int n : threads) {
        cout << "Ejecutando con " << n << " threads...\n";
        // Usar un archivo temporal para el índice
        string cmd = indiceProgram + " " + to_string(n) + " " + idxTemp + " --auto > /dev/null";
        
        auto start = chrono::high_resolution_clock::now();
        int ret = system(cmd.c_str());
        auto end = chrono::high_resolution_clock::now();
        
        if (ret != 0) {
            cerr << "Error ejecutando " << cmd << "\n";
            continue;
        }

        long long duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
        logFile << n << "," << duration << "\n";
        cout << "Tiempo: " << duration << " ms\n";
    }
    logFile.close();

    cout << "Generando gráfico...\n";
    string pyCmd = "python3 " + pythonScript + " " + logPath;
    int pyRet = system(pyCmd.c_str());
    
    if (pyRet == 0) {
        cout << "Gráfico generado exitosamente.\n";
    } else {
        cerr << "Error generando el gráfico.\n";
    }

    cout << "Fin del proceso. Presione Enter para volver.";
    cin.get();

    return 0;
}
