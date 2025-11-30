#include "../include/motor.h"

// Variables globales del motor
static unordered_map<string, string> mapaLibros;  // id -> nombre real del libro
static unordered_map<string, vector<pair<string, int>>> indiceInvertido;  // palabra -> [(id, freq), ...]

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

// Carga el mapa de libros (formato: id; nombre)
bool cargarMapaLibros(const string& rutaMapa) {
    cout << "Motor: Intentando abrir mapa: " << rutaMapa << "\n";
    
    ifstream f(rutaMapa);
    if (!f.is_open()) {
        cerr << "Motor: No se pudo abrir mapa de libros: " << rutaMapa << "\n";
        return false;
    }

    string linea;
    int count = 0;
    while (getline(f, linea)) {
        if (linea.empty()) continue;
        size_t pos = linea.find(';');
        if (pos == string::npos) continue;

        string id = linea.substr(0, pos);
        string nombre = linea.substr(pos + 1);

        // trim espacios
        auto trim = [](string& s) {
            size_t a = s.find_first_not_of(" \t\r\n");
            size_t b = s.find_last_not_of(" \t\r\n");
            if (a == string::npos) { s.clear(); return; }
            s = s.substr(a, b - a + 1);
        };
        trim(id);
        trim(nombre);

        if (!id.empty() && !nombre.empty()) {
            mapaLibros[id] = nombre;
            count++;
        }
    }
    cout << "Motor: Cargados " << count << " libros en mapa\n";
    return true;
}

// Carga el índice invertido (formato: palabra;(id1,freq1);(id2,freq2);...)
bool cargarIndiceInvertido(const string& rutaIndice) {
    cout << "Motor: Intentando abrir índice: " << rutaIndice << "\n";
    
    ifstream f(rutaIndice);
    if (!f.is_open()) {
        cerr << "Motor: No se pudo abrir índice: " << rutaIndice << "\n";
        return false;
    }

    string linea;
    int count = 0;
    while (getline(f, linea)) {
        if (linea.empty()) continue;

        size_t pos = linea.find(';');
        if (pos == string::npos) continue;

        string palabra = linea.substr(0, pos);
        string resto = linea.substr(pos + 1);

        vector<pair<string, int>> docs;

        // Parsear formato (id,freq);(id,freq);...
        size_t inicio = 0;
        while ((inicio = resto.find('(', inicio)) != string::npos) {
            size_t fin = resto.find(')', inicio);
            if (fin == string::npos) break;

            string par = resto.substr(inicio + 1, fin - inicio - 1);
            size_t coma = par.find(',');
            if (coma != string::npos) {
                string id = par.substr(0, coma);
                try {
                    int freq = stoi(par.substr(coma + 1));
                    docs.push_back({id, freq});
                } catch (const exception& e) {
                    cerr << "Motor: Error parseando freq en línea: " << linea << "\n";
                }
            }
            inicio = fin + 1;
        }

        if (!docs.empty()) {
            indiceInvertido[palabra] = docs;
            count++;
        }
    }
    cout << "Motor: Cargadas " << count << " palabras en índice\n";
    return true;
}

// Tokeniza una frase en palabras individuales
static vector<string> tokenizar(const string& frase) {
    vector<string> palabras;
    string palabra;
    
    for (char c : frase) {
        if (isalnum(static_cast<unsigned char>(c))) {
            palabra += tolower(static_cast<unsigned char>(c));
        } else if (!palabra.empty()) {
            palabras.push_back(palabra);
            palabra.clear();
        }
    }
    if (!palabra.empty()) {
        palabras.push_back(palabra);
    }
    
    return palabras;
}

// Busca una o más palabras y devuelve JSON con los TOP K resultados
// Si hay múltiples palabras, suma los scores por libro
string buscarPalabra(const string& consulta, int topK) {
    auto t0 = chrono::high_resolution_clock::now();

    // Tokenizar la consulta en palabras individuales
    vector<string> palabras = tokenizar(consulta);
    
    if (palabras.empty()) {
        return R"({"query":")" + consulta + R"(","origen_respuesta":"motor","tiempo_motor_us":0,"topK":)" 
             + to_string(topK) + R"(,"respuesta":[],"mensaje":"Consulta vacía"})";
    }

    // Acumular scores por libro (id -> score total)
    unordered_map<string, int> scoresAcumulados;
    int palabrasEncontradas = 0;

    for (const string& palabra : palabras) {
        auto it = indiceInvertido.find(palabra);
        if (it != indiceInvertido.end()) {
            palabrasEncontradas++;
            for (const auto& [id, freq] : it->second) {
                scoresAcumulados[id] += freq;
            }
        }
    }

    if (scoresAcumulados.empty()) {
        auto t1 = chrono::high_resolution_clock::now();
        long tiempoMotor_us = chrono::duration_cast<chrono::microseconds>(t1 - t0).count();
        
        return R"({"query":")" + consulta + R"(","origen_respuesta":"motor","tiempo_motor_us":)" 
             + to_string(tiempoMotor_us) + R"(,"topK":)" 
             + to_string(topK) + R"(,"respuesta":[],"mensaje":"Ninguna palabra encontrada"})";
    }

    // Convertir a vector y ordenar por score descendente
    vector<pair<string, int>> resultados(scoresAcumulados.begin(), scoresAcumulados.end());
    sort(resultados.begin(), resultados.end(),
         [](const auto& a, const auto& b) { return a.second > b.second; });

    // Tomar top K
    if ((int)resultados.size() > topK) {
        resultados.resize(topK);
    }

    auto t1 = chrono::high_resolution_clock::now();
    long tiempoMotor_us = chrono::duration_cast<chrono::microseconds>(t1 - t0).count();

    // Construir JSON
    string json = "{\n";
    json += "  \"query\": \"" + consulta + "\",\n";
    json += "  \"origen_respuesta\": \"motor\",\n";
    json += "  \"tiempo_motor_us\": " + to_string(tiempoMotor_us) + ",\n";
    json += "  \"palabras_buscadas\": " + to_string(palabras.size()) + ",\n";
    json += "  \"palabras_encontradas\": " + to_string(palabrasEncontradas) + ",\n";
    json += "  \"topK\": " + to_string(topK) + ",\n";
    json += "  \"respuesta\": [\n";

    for (size_t i = 0; i < resultados.size(); ++i) {
        string id = resultados[i].first;
        int score = resultados[i].second;

        // Mapeo inverso: id -> nombre real del libro
        string nombreLibro = id;  // fallback si no está en mapa
        auto itMapa = mapaLibros.find(id);
        if (itMapa != mapaLibros.end()) {
            nombreLibro = itMapa->second;
        }

        // Escapar comillas en el nombre del libro
        string nombreEscapado;
        for (char c : nombreLibro) {
            if (c == '"') nombreEscapado += "\\\"";
            else nombreEscapado += c;
        }

        json += "    {\"libro\": \"" + nombreEscapado + "\", \"score\": " + to_string(score) + "}";
        if (i < resultados.size() - 1) json += ",";
        json += "\n";
    }

    json += "  ]\n}";
    return json;
}

void iniciarServidorMotor() {
    // Leer variables de entorno
    const char* envPort = getenv("MOTOR_PORT");
    const char* envTopK = getenv("TOPK");
    const char* envIndice = getenv("INDICE_PATH");
    const char* envMapa = getenv("MAPA_LIBROS");

    // Validar que todas las variables existen
    if (!envPort || !envTopK || !envIndice || !envMapa) {
        cerr << "Motor: Error - faltan variables de entorno requeridas:\n";
        if (!envPort)   cerr << "  - MOTOR_PORT\n";
        if (!envTopK)   cerr << "  - TOPK\n";
        if (!envIndice) cerr << "  - INDICE_PATH\n";
        if (!envMapa)   cerr << "  - MAPA_LIBROS\n";
        cerr << "\nAsegúrate de exportar las variables o cargar el .env\n";
        return;
    }

    int motorPort = atoi(envPort);
    int topK = atoi(envTopK);
    string rutaIndice = envIndice;
    string rutaMapa = envMapa;

    cout << "Motor: Configuración:\n";
    cout << "  Puerto: " << motorPort << "\n";
    cout << "  TopK: " << topK << "\n";
    cout << "  Índice: " << rutaIndice << "\n";
    cout << "  Mapa: " << rutaMapa << "\n\n";

    // Verificar que los archivos existen antes de intentar cargarlos
    if (access(rutaMapa.c_str(), F_OK) != 0) {
        cerr << "Motor: Advertencia - archivo mapa no existe: " << rutaMapa << "\n";
    } else {
        cargarMapaLibros(rutaMapa);
    }

    if (access(rutaIndice.c_str(), F_OK) != 0) {
        cerr << "Motor: Error - archivo índice no existe: " << rutaIndice << "\n";
        cerr << "Motor: No se puede continuar sin índice.\n";
        return;
    }
    
    if (!cargarIndiceInvertido(rutaIndice)) {
        cerr << "Motor: Error fatal - no se pudo cargar índice\n";
        return;
    }

    // Crear socket servidor
    int serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0) {
        cerr << "Motor: Error al crear socket: " << strerror(errno) << "\n";
        return;
    }

    int opt = 1;
    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        cerr << "Motor: Error en setsockopt: " << strerror(errno) << "\n";
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(motorPort);

    if (bind(serverFd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        cerr << "Motor: Error en bind: " << strerror(errno) << "\n";
        close(serverFd);
        return;
    }

    if (listen(serverFd, 10) < 0) {
        cerr << "Motor: Error en listen: " << strerror(errno) << "\n";
        close(serverFd);
        return;
    }

    cout << "Motor: Escuchando en puerto " << motorPort << "...\n\n";

    // Loop principal - atender consultas
    while (true) {
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);
        int clientFd = accept(serverFd, (sockaddr*)&clientAddr, &clientLen);
        if (clientFd < 0) {
            cerr << "Motor: Error en accept: " << strerror(errno) << "\n";
            continue;
        }

        char buffer[4096] = {0};
        ssize_t n = read(clientFd, buffer, 4095);
        if (n > 0) {
            string palabra(buffer, n);  // usar n para evitar basura
            // Quitar newline si existe
            while (!palabra.empty() && (palabra.back() == '\n' || palabra.back() == '\r' || palabra.back() == '\0')) {
                palabra.pop_back();
            }

            // Convertir a minúsculas para buscar
            transform(palabra.begin(), palabra.end(), palabra.begin(), ::tolower);

            cout << "Motor: Buscando '" << palabra << "'...\n";

            string resultado = buscarPalabra(palabra, topK);

            cout << "Motor: Enviando respuesta (" << resultado.size() << " bytes)\n\n";

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
    cout << " Motor de Búsqueda\n";
    cout << " Motor - PID: " << getpid() << "\n";
    cout << "==============================\n\n";

    iniciarServidorMotor();

    return 0;
}