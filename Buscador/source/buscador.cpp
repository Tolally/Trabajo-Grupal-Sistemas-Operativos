#include "../include/buscador.h"

void limpiarConsola(){
    #ifdef _WIN32
        if (system("cls") != 0) { /* ignorar error */ }
    #else
        if (system("clear") != 0) { /* ignorar error */ }
    #endif
}

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

string buscar(const string& palabra){
    const char* portEnv = getenv("CACHE_PORT");
    int cache_port = atoi(portEnv);

    int sockFd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockFd < 0) {
        return "{\"error\": \"No se pudo crear socket\"}";
    }

    sockaddr_in cacheAddr{};
    cacheAddr.sin_family = AF_INET;
    cacheAddr.sin_port = htons(cache_port);
    inet_pton(AF_INET, "127.0.0.1", &cacheAddr.sin_addr);

    if (connect(sockFd, (sockaddr*)&cacheAddr, sizeof(cacheAddr)) < 0) {
        close(sockFd);
        return "{\"error\": \"No se pudo conectar a cache (¿está corriendo?)\"}";
    }

    ssize_t written = write(sockFd, palabra.c_str(), palabra.size());

    if (written < 0) {
        close(sockFd);
        return "{\"error\": \"Error al enviar consulta\"}";
    }

    char buffer[4096] = {0};
    ssize_t n = read(sockFd, buffer, 4095);
    close(sockFd);

    if (n > 0) {
        return string(buffer);  // ya viene como JSON desde cache/motor
    }
    return "{\"error\": \"Sin respuesta\"}";
}

void iniciarBuscador() {
    cout << "=================================\n";
    cout << "  Escriba una palabra para buscar\n";
    cout << "  '0' para volver al menú\n";
    cout << "=================================\n\n";

    string palabra;
    while (true) {
        cout << "Buscar: ";
        if (!getline(cin, palabra)) break;
        if (palabra == "0") break;
        if (palabra.empty()) continue;

        string resultado = buscar(palabra);
        cout << "\nResultado:\n" << resultado << "\n\n";
    }
    limpiarConsola();
}

int main() {
    cargarDotEnvSiExiste();
    string opcion;

    while (true) {
        cout << "===================================\n";
        cout << " PID del proceso actual: " << getpid() << endl;
        cout << "\n===================================\n";
        cout << "           MENÚ BUSCADOR\n";
        cout << "===================================\n";
        cout << "1. Iniciar búsqueda\n";
        cout << "0. Salir\n";
        cout << "Opción: ";

        if (!getline(cin, opcion)) break;

        if (opcion == "0") {
            break;
        } else if (opcion == "1") {
            iniciarBuscador();
        } else {
            cout << "Opción no válida.\n";
        }
    }

    cout << "Buscador finalizado.\n";
    return 0;
}