#include "../include/cache.h"

// Cache en memoria: palabra -> resultado JSON
static unordered_map<string, string> cacheData;

// Consulta al servidor motor
static string consultarMotor(const string& palabra){
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

void iniciarServidorCache(){
    int serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0) {
        cerr << "Cache: Error al crear socket\n";
        return;
    }

    int opt = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    const char* portEnv = getenv("CACHE_PORT");
    int cache_port = atoi(portEnv);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(cache_port);

    
    if (bind(serverFd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        cerr << "Cache: Error en bind\n";
        close(serverFd);
        return;
    }

    if (listen(serverFd, 5) < 0) {
        cerr << "Cache: Error en listen\n";
        close(serverFd);
        return;
    }

    cout << "Cache: Escuchando en puerto " << cache_port << "\n";

    while(true){
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);
        int clientFd = accept(serverFd, (sockaddr*)&clientAddr, &clientLen);
        if (clientFd<0){
            cerr << "Cache: Error en accept\n";
            continue;
        }

        char buffer[4096] = {0};
        ssize_t n = read(clientFd, buffer, 4095);

        if (n > 0){
            string palabra(buffer);
            if(!palabra.empty() && palabra.back() == '\n')
                palabra.pop_back();
            
            cout << "Cache: Consulta '"<<palabra<<"'\n";

            string resultado;
            auto it = cacheData.find(palabra);
            if (it != cacheData.end()){
                cout << "Encontrado en Cache\n";
                resultado = it -> second;
            } else{
                cout << "No encontrado en cache - Consultando a Motor\n";
                resultado = consultarMotor(palabra);
                // guardar solo si no es error
                if (resultado.find("\"error\"") == string::npos)
                    cacheData[palabra] = resultado;
            }
            ssize_t w = write(clientFd, resultado.c_str(), resultado.size());
            (void)w;  // ignorar warning
        }

        close(clientFd);
    }

    close(serverFd);
}

int main() {
    cout << "===================================\n";
    cout << " Cache - PID: " << getpid() << endl;
    cout << "===================================\n";
    iniciarServidorCache();
    return 0;
}