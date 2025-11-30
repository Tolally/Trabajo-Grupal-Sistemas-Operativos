#include "../include/cache.h"

// Cache en memoria: palabra -> resultado JSON
static unordered_map<string, string> cacheData;
// Cola FIFO para saber el orden de inserción
static queue<string> cacheOrder;

int cacheSize;

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

// Consulta al servidor motor
static string consultarMotor(const string& palabra) {
    int sockFd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockFd < 0) {
        return "{\"error\": \"No se pudo crear socket a motor\"}";
    }

    const char* portEnv = getenv("MOTOR_PORT");
    int motor_port = atoi(portEnv);

    sockaddr_in motorAddr{};
    motorAddr.sin_family = AF_INET;
    motorAddr.sin_port = htons(motor_port);
    inet_pton(AF_INET, "127.0.0.1", &motorAddr.sin_addr);

    if (connect(sockFd, (sockaddr*)&motorAddr, sizeof(motorAddr)) < 0) {
        close(sockFd);
        return "{\"error\": \"No se pudo conectar a motor\"}";
    }

    ssize_t written = write(sockFd, palabra.c_str(), palabra.size());
    if (written < 0) {
        close(sockFd);
        return "{\"error\": \"Error al enviar a motor\"}";
    }

    char buffer[4096] = {0};
    ssize_t n = read(sockFd, buffer, 4095);
    close(sockFd);

    if (n > 0) {
        return string(buffer);
    }
    return "{\"error\": \"Sin respuesta de motor\"}";
}

// Inserta en cache con política FIFO
static void insertarEnCache(const string& palabra, const string& resultado) {
    // Si ya existe, no duplicar (actualizar valor solamente)
    if (cacheData.find(palabra) != cacheData.end()) {
        cacheData[palabra] = resultado;
        return;
    }

    // Si el cache está lleno, eliminar el más antiguo (FIFO)
    while ((int)cacheData.size() >= cacheSize && !cacheOrder.empty()) {
        string antiguo = cacheOrder.front();
        cacheOrder.pop();
        cacheData.erase(antiguo);
        cout << "Cache: Eliminado (FIFO) '" << antiguo << "'\n";
    }

    // Insertar nuevo
    cacheData[palabra] = resultado;
    cacheOrder.push(palabra);
    cout << "Cache: Insertado '" << palabra << "' (" << cacheData.size() << "/" << cacheSize << ")\n";
}

// Agrega tiempo_cache_us y tiempo_total_us al JSON, y cambia origen si es HIT
static string agregarTiemposAlJSON(const string& json, long tiempoCache_us, long tiempoMotor_us, bool esHit) {
    string resultado = json;
    
    // Cambiar origen_respuesta si es HIT
    if (esHit) {
        size_t pos = resultado.find("\"origen_respuesta\": \"motor\"");
        if (pos != string::npos) {
            resultado.replace(pos, 27, "\"origen_respuesta\": \"cache\"");
        }
    }
    
    // Buscar donde insertar tiempo_cache_us (después de tiempo_motor_us)
    size_t posMotor = resultado.find("\"tiempo_motor_us\":");
    if (posMotor != string::npos) {
        // Encontrar el final del valor de tiempo_motor_us
        size_t finValor = resultado.find(",", posMotor);
        if (finValor != string::npos) {
            // Insertar tiempo_cache_us y tiempo_total_us después
            long tiempoTotal = tiempoCache_us + tiempoMotor_us;
            string tiemposExtra = ",\n  \"tiempo_cache_us\": " + to_string(tiempoCache_us) +
                                  ",\n  \"tiempo_total_us\": " + to_string(tiempoTotal);
            resultado.insert(finValor, tiemposExtra);
        }
    }
    
    return resultado;
}

void iniciarServidorCache() {
    // Leer variables de entorno
    const char* portEnv = getenv("CACHE_PORT");
    const char* sizeEnv = getenv("CACHE_SIZE");

    int cache_port = atoi(portEnv);
    cacheSize = atoi(sizeEnv);

    cout << "Cache: Configuración:\n";
    cout << "  Puerto: " << cache_port << "\n";
    cout << "  Tamaño máximo: " << cacheSize << " entradas\n\n";

    int serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0) {
        cerr << "Cache: Error al crear socket\n";
        return;
    }

    int opt = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(cache_port);

    if (bind(serverFd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        cerr << "Cache: Error en bind\n";
        close(serverFd);
        return;
    }

    if (listen(serverFd, 10) < 0) {
        cerr << "Cache: Error en listen\n";
        close(serverFd);
        return;
    }

    cout << "Cache: Escuchando en puerto " << cache_port << "...\n\n";

    while (true) {
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);
        int clientFd = accept(serverFd, (sockaddr*)&clientAddr, &clientLen);
        if (clientFd < 0) {
            cerr << "Cache: Error en accept\n";
            continue;
        }

        char buffer[4096] = {0};
        ssize_t n = read(clientFd, buffer, 4095);

        if (n > 0) {
            string palabra(buffer);
            // Limpiar newlines
            if (!palabra.empty() && palabra.back() == '\n') palabra.pop_back();
            if (!palabra.empty() && palabra.back() == '\r') palabra.pop_back();

            // Convertir a minúsculas para normalizar la consulta
            transform(palabra.begin(), palabra.end(), palabra.begin(), ::tolower);

            cout << "Cache: Consulta '" << palabra << "'\n";

            // Medir tiempo de búsqueda en cache
            auto t0 = chrono::high_resolution_clock::now();
            
            string resultado;
            auto it = cacheData.find(palabra);
            
            auto t1 = chrono::high_resolution_clock::now();
            long tiempoCache_us = chrono::duration_cast<chrono::microseconds>(t1 - t0).count();

            if (it != cacheData.end()) {
                // CACHE HIT
                cout << "Cache: HIT (" << tiempoCache_us << " us)\n";
                resultado = it->second;
                
                // Agregar tiempos (tiempo_motor_us = 0 porque no se consultó)
                resultado = agregarTiemposAlJSON(resultado, tiempoCache_us, 0, true);
                
            } else {
                // CACHE MISS - consultar motor
                cout << "Cache: MISS -> consultando motor\n";
                
                resultado = consultarMotor(palabra);
                
                // El tiempo del motor ya viene en el JSON, extraerlo
                long tiempoMotor_us = 0;
                size_t posMotor = resultado.find("\"tiempo_motor_us\":");
                if (posMotor != string::npos) {
                    size_t inicio = posMotor + 18; // largo de "tiempo_motor_us":
                    size_t fin = resultado.find_first_of(",}", inicio);
                    if (fin != string::npos) {
                        string valorStr = resultado.substr(inicio, fin - inicio);
                        // trim espacios
                        while (!valorStr.empty() && isspace(valorStr.front())) valorStr.erase(0, 1);
                        tiempoMotor_us = stol(valorStr);
                    }
                }

                // Guardar solo si no es error
                if (resultado.find("\"error\"") == string::npos) {
                    insertarEnCache(palabra, resultado);
                }
                
                // Agregar tiempos al JSON
                resultado = agregarTiemposAlJSON(resultado, tiempoCache_us, tiempoMotor_us, false);
            }

            ssize_t w = write(clientFd, resultado.c_str(), resultado.size());
            (void)w;  // ignorar warning
        }

        close(clientFd);
    }

    close(serverFd);
}

int main() {
    cargarDotEnvSiExiste();

    cout << "==============================\n";
    cout << " Cache (FIFO)\n";
    cout << " Cache - PID: " << getpid() << "\n";
    cout << "==============================\n\n";

    iniciarServidorCache();
    return 0;
}