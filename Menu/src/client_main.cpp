#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <atomic>
#include <chrono>
#include <cstring>
#include <functional>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <iomanip>

#include "../include/game_protocol.h"

using namespace std;

atomic<bool> running(true);
atomic<bool> myTurn(false);
mutex cout_m;
mutex state_m;

// Estado de la interfaz
string myTeam = "";
string username = "";
string currentScores = "";
string currentTurnInfo = "";
string lastRollInfo = "";  // Guardar info del último roll
string lobbyInfo = "";
bool gameStarted = false;

bool send_line(int sock, const string& s) {
    string out = s + "\n";
    ssize_t total = 0;
    const char* data = out.c_str();
    ssize_t towrite = out.size();
    while (total < towrite) {
        ssize_t w = write(sock, data + total, towrite - total);
        if (w <= 0) return false;
        total += w;
    }
    return true;
}

bool recv_line(int sock, string& out) {
    out.clear();
    char c;
    while (true) {
        ssize_t r = read(sock, &c, 1);
        if (r <= 0) return false;
        if (c == '\n') break;
        out.push_back(c);
    }
    return true;
}

void clear_screen() {
    // Usar el comando del sistema operativo para limpiar
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void redraw_interface() {
    clear_screen();
    
    // Header
    cout << "╔══════════════════════════════════════╗" << endl;
    cout << "║       🎮 JUEGO MULTIPLAYER 🎮        ║" << endl;
    cout << "╠══════════════════════════════════════╣" << endl;
    cout << "║ Jugador: " << left << setw(26) << username << "║" << endl;
    
    if (!myTeam.empty()) {
        cout << "║ Equipo: " << left << setw(27) << myTeam << "║" << endl;
    }
    
    cout << "╚══════════════════════════════════════╝" << endl;
    cout << endl;
    
    // Mostrar información del lobby si NO estamos en juego
    if (!gameStarted && !lobbyInfo.empty()) {
        cout << "👥 Sala: " << lobbyInfo << endl << endl;
    }
    // Mostrar puntuaciones si el juego ha iniciado
    if (gameStarted && !currentScores.empty()) {
        cout << currentScores << endl;
    } else if (gameStarted) {
        cerr << "[DEBUG redraw] gameStarted=true pero currentScores está vacío!" << endl;
    }
    
    // Mostrar información del último roll
    if (!lastRollInfo.empty()) {
        cout << lastRollInfo << endl << endl;
    }
    
    // Mostrar información del turno
    if (!currentTurnInfo.empty()) {
        cout << currentTurnInfo << endl << endl;
    }
    
    // Prompt
    if (myTurn) {
        cout << "🎲 ¡ES TU TURNO!" << endl;
        cout << ">>> Escribe 'r' para lanzar > ";
    } else if (gameStarted) {
        cout << ">>> Esperando... > ";
    } else {
        cout << ">>> Comandos: 'ready <equipo>' | 'quit' > ";
    }
    
    cout.flush();
}

void reader_thread_func(int sock) {
    string line;
    while (recv_line(sock, line)) {
        vector<string> parts;
        string cur;
        for (char ch : line) {
            if (ch == SEP) { parts.push_back(cur); cur.clear(); }
            else cur.push_back(ch);
        }
        if (!cur.empty()) parts.push_back(cur);
        string type = parts.size()>0?parts[0]:"";
        
        bool shouldRedraw = false;
        
        {
            lock_guard<mutex> lk(state_m);
            
            if (type == "WELCOME") {
                lobbyInfo = "Conectado al servidor";
                shouldRedraw = true;
            } 
            else if (type == "LOBBY") {
                // SIEMPRE actualizar lobby info
                if (parts.size() > 1) {
                    lobbyInfo = parts[1];
                    // Solo redibujar si NO estamos en juego
                    if (!gameStarted) {
                        shouldRedraw = true;
                    }
                }
            }
            else if (type == "START") {
                gameStarted = true;
                currentTurnInfo = "🎉 ¡JUEGO INICIADO!";
                if (parts.size() > 1) {
                    currentTurnInfo += "\n   Meta: " + parts[1] + " puntos";
                }
                if (parts.size() > 2) {
                    currentTurnInfo += " | Dado: " + parts[2] + " caras";
                }
                lastRollInfo = "";
                shouldRedraw = true;
            }
            else if (type == "SCORES") {
                if (parts.size() > 1) {
                    string rawScores = parts[1];
                    
                    // IMPORTANTE: Decodificar los saltos de línea
                    size_t pos = 0;
                    while ((pos = rawScores.find("\\n", pos)) != string::npos) {
                        rawScores.replace(pos, 2, "\n");  // Reemplazar \\n con \n real
                        pos += 1;
                    }
                    
                    // Procesar las puntuaciones y marcar "tu equipo"
                    stringstream ss(rawScores);
                    stringstream output;
                    string line;
                    
                    while (getline(ss, line)) {
                        if (line.empty()) {
                            output << line << "\n";
                            continue;
                        }
                        
                        // Si la línea contiene nuestro equipo, agregar "(tu equipo)"
                        if (!myTeam.empty() && line.find("Equipo " + myTeam) != string::npos) {
                            size_t pos = line.find(":");
                            if (pos != string::npos) {
                                output << line.substr(0, pos) << " (tu equipo)" << line.substr(pos) << "\n";
                            } else {
                                output << line << "\n";
                            }
                        } else {
                            output << line << "\n";
                        }
                    }
                    
                    currentScores = output.str();
                    cerr << "[DEBUG] currentScores guardado, longitud: " << currentScores.length() << endl;
                    shouldRedraw = true;
                } else {
                    cerr << "[DEBUG] SCORES sin contenido (parts.size = " << parts.size() << ")" << endl;
                }
            }
            else if (type == "YOURTURN") {
                myTurn = true;
                currentTurnInfo = "🎯 ¡Tu turno de jugar!";
                shouldRedraw = true;
            } 
            else if (type == "TURN_INFO") {
                myTurn = false;
                string turnInfo = parts.size()>1?parts[1]:"";
                shouldRedraw = true;
            }
            else if (type == "ROLLING") {
                currentTurnInfo = "🎲 Lanzando el dado...";
                shouldRedraw = true;
            } 
            else if (type == "ROLL_RESULT") {
                string result = parts.size()>1?parts[1]:"";
                
                // Guardar el resultado del roll
                if (result.find("Sacaste") != string::npos) {
                    // Es nuestro resultado
                    lastRollInfo = "📊 Último turno: " + result;
                } else if (result.find("sacó") != string::npos) {
                    // Es resultado de otro jugador/equipo
                    lastRollInfo = "📊 " + result;
                }
                
                myTurn = false;
                currentTurnInfo = "";
                shouldRedraw = true;
            } 
            else if (type == "END") {
                gameStarted = false;
                myTurn = false;
                currentTurnInfo = "🏆 ¡JUEGO TERMINADO!\n   Ganador: " + (parts.size()>1?parts[1]:"");
                currentScores = "";
                lastRollInfo = "";
                shouldRedraw = true;
                
                // Mostrar resultado y salir
                lock_guard<mutex> lk_cout(cout_m);
                redraw_interface();
                this_thread::sleep_for(chrono::seconds(3));
                running = false;
                return;
            } 
            else if (type == "PLAYER_LEFT") {
                string info = parts.size()>1?parts[1]:"";
                size_t sep_pos = info.find('|');
                if (sep_pos != string::npos) {
                    string playerName = info.substr(0, sep_pos);
                    string teamName = info.substr(sep_pos + 1);
                    currentTurnInfo = "❌ " + playerName + " del equipo " + teamName + " se desconectó";
                    shouldRedraw = true;
                }
            } 
            else if (type == "ERROR") {
                currentTurnInfo = "⚠️  Error: " + (parts.size()>1?parts[1]:"");
                shouldRedraw = true;
            }
        }
        
        // Redibujar si es necesario
        if (shouldRedraw) {
            lock_guard<mutex> lk_cout(cout_m);
            redraw_interface();
        }
    }
    running = false;
}

int main(int argc, char** argv) {
    if (argc < 4) {
        cerr << "Uso: " << argv[0] << " <host> <port> <username>" << endl;
        return 1;
    }
    
    string host = argv[1];
    int port = stoi(argv[2]);
    username = argv[3];

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); return 1; }
    
    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, host.c_str(), &serv_addr.sin_addr) <= 0) {
        cerr << "Dirección inválida" << endl; 
        return 1;
    }
    
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect"); 
        return 1;
    }

    cout << "Conectando al servidor..." << endl;
    
    send_line(sock, make_msg2("JOIN", username));
    
    thread reader(reader_thread_func, sock);
    
    // Pequeño delay para recibir el WELCOME
    this_thread::sleep_for(chrono::milliseconds(300));
    {
        lock_guard<mutex> lk(cout_m);
        redraw_interface();
    }

    while (running) {
        string line;
        if (!getline(cin, line)) break;

        auto trim = [](string &s){
            while (!s.empty() && isspace((unsigned char)s.front())) s.erase(s.begin());
            while (!s.empty() && isspace((unsigned char)s.back())) s.pop_back();
        };
        trim(line);
        if (line.empty()) {
            lock_guard<mutex> lk(cout_m);
            redraw_interface();
            continue;
        }

        if (myTurn) {
            char c = tolower((unsigned char)line[0]);
            if (c == 'r') {
                if (!send_line(sock, make_msg("ROLL"))) {
                    lock_guard<mutex> lk(state_m);
                    currentTurnInfo = "⚠️  Error enviando ROLL";
                    running = false;
                    break;
                }
                // No redibujar aquí, esperar respuesta del servidor
            } else if (c == 'q') {
                send_line(sock, make_msg("QUIT"));
                running = false;
                break;
            } else {
                lock_guard<mutex> lk_state(state_m);
                lock_guard<mutex> lk_cout(cout_m);
                currentTurnInfo = "⚠️  Comando no reconocido. Usa 'r' para lanzar";
                redraw_interface();
            }
        } else {
            stringstream ss(line);
            string cmd; 
            ss >> cmd;
            
            if (cmd == "ready") {
                string team; 
                ss >> team;
                if (team.empty()) {
                    lock_guard<mutex> lk_state(state_m);
                    lock_guard<mutex> lk_cout(cout_m);
                    currentTurnInfo = "⚠️  Debes especificar un nombre de equipo";
                    redraw_interface();
                    continue;
                }
                
                {
                    lock_guard<mutex> lk(state_m);
                    myTeam = team;
                    lobbyInfo = "Uniéndose al equipo " + team + "...";
                }
                
                send_line(sock, make_msg2("READY", team));
                
                // Redibujar para mostrar el cambio
                this_thread::sleep_for(chrono::milliseconds(50));
                lock_guard<mutex> lk(cout_m);
                redraw_interface();
                
            } else if (cmd == "quit") {
                send_line(sock, make_msg("QUIT"));
                running = false;
                break;
            } else {
                lock_guard<mutex> lk_state(state_m);
                lock_guard<mutex> lk_cout(cout_m);
                currentTurnInfo = "⚠️  Comando no reconocido. Usa 'ready <equipo>' o 'quit'";
                redraw_interface();
            }
        }
    }

    close(sock);
    if (reader.joinable()) reader.join();
    
    clear_screen();
    cout << "👋 Cliente finalizado" << endl;
    return 0;
}