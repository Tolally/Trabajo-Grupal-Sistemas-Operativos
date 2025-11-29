#include "../include/buscador.h"

void limpiarConsola(){
    #ifdef _WIN32
        if (system("cls") != 0) { /* ignorar error */ }
    #else
        if (system("clear") != 0) { /* ignorar error */ }
    #endif
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