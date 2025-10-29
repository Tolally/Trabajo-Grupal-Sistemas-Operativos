#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fstream>
#include <algorithm>
#include <cerrno>
#include <csignal>
#include <cstring>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <iomanip>

#include "../include/game_protocol.h"
#include "../include/game.h"

using namespace std;

static Game* G = nullptr;
static mutex send_mtx;
static mutex sock_mtx;
static map<int,int> id_to_sock;
static map<int, string> id_to_team;

void load_env_variables() {
    ifstream env_file(".env");
    string line;
    while (getline(env_file, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        size_t pos = line.find('=');
        if (pos != string::npos) {
            string key = line.substr(0, pos);
            string value = line.substr(pos + 1);
            setenv(key.c_str(), value.c_str(), 1);
        }
    }
}

string get_timestamp() {
    auto now = chrono::system_clock::now();
    auto time = chrono::system_clock::to_time_t(now);
    stringstream ss;
    ss << put_time(localtime(&time), "%H:%M:%S");
    return ss.str();
}

void log_info(const string& msg) {
    cerr << "[" << get_timestamp() << "] ℹ️  " << msg << endl;
}

void log_event(const string& msg) {
    cerr << "[" << get_timestamp() << "] 📋 " << msg << endl;
}

void log_game(const string& msg) {
    cerr << "[" << get_timestamp() << "] 🎮 " << msg << endl;
}

void log_error(const string& msg) {
    cerr << "[" << get_timestamp() << "] ⚠️  " << msg << endl;
}

bool send_line_raw(int sock, const string& s) {
    lock_guard<mutex> lk(send_mtx);
    string data = s + "\n";
    const char* buf = data.c_str();
    size_t towrite = data.size();
    size_t total = 0;
    while (total < towrite) {
        ssize_t w = write(sock, buf + total, towrite - total);
        if (w <= 0) return false;
        total += w;
    }
    return true;
}

bool recv_line_raw(int sock, string& out) {
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

void send_to_id(int id, const string& msg) {
    lock_guard<mutex> lk(sock_mtx);
    auto it = id_to_sock.find(id);
    if (it == id_to_sock.end()) return;
    send_line_raw(it->second, msg);
}

void broadcast_msg(const string& msg) {
    lock_guard<mutex> lk(sock_mtx);
    for (auto &kv : id_to_sock) {
        send_line_raw(kv.second, msg);
    }
}

void send_to_team_except(const string& teamName, int exceptId, const string& msg) {
    lock_guard<mutex> lk(sock_mtx);
    for (auto &kv : id_to_sock) {
        int playerId = kv.first;
        if (playerId == exceptId) continue;
        
        auto teamIt = id_to_team.find(playerId);
        if (teamIt != id_to_team.end() && teamIt->second == teamName) {
            send_line_raw(kv.second, msg);
        }
    }
}

void send_to_other_teams(const string& excludedTeam, const string& msg) {
    lock_guard<mutex> lk(sock_mtx);
    for (auto &kv : id_to_sock) {
        int playerId = kv.first;
        auto teamIt = id_to_team.find(playerId);
        if (teamIt != id_to_team.end() && teamIt->second != excludedTeam) {
            send_line_raw(kv.second, msg);
        }
    }
}

void broadcast_scores_to_all() {
    string scores_display = G->build_scores_display();
    log_game("Enviando puntuaciones a todos los jugadores:");
    cerr << scores_display << endl;
    
    // IMPORTANTE: Codificar los saltos de línea como un marcador especial
    string encoded = scores_display;
    size_t pos = 0;
    while ((pos = encoded.find('\n', pos)) != string::npos) {
        encoded.replace(pos, 1, "\\n");  // Reemplazar \n con \\n (dos caracteres)
        pos += 2;
    }
    
    broadcast_msg(make_msg2("SCORES", encoded));
}

void broadcast_board_update() {
    string board_display = G->build_board_display();
    cerr << endl << board_display << endl;
}

void handle_turn_notification() {
    int currentId = G->current_player_id();
    if (currentId == -1) return;
    
    string currentTeam = G->get_player_team(currentId);
    string currentPlayerName = G->get_player_name(currentId);
    
    log_game("Turno de " + currentPlayerName + " (Equipo " + currentTeam + ")");
    
    send_to_id(currentId, make_msg2("YOURTURN", "¡ES TU TURNO! Escribe 'r' para lanzar el dado"));
    
    send_to_team_except(currentTeam, currentId, 
        make_msg2("TURN_INFO", "Es el turno de tu compañero " + currentPlayerName));
    
    send_to_other_teams(currentTeam, 
        make_msg2("TURN_INFO", "Es el turno del equipo " + currentTeam));
}

void handle_client(int client_sock) {
    int myId = -1;
    string myTeam = "";
    string myName = "";
    string line;
    
    // Log de nueva conexión
    sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    getpeername(client_sock, (struct sockaddr*)&addr, &addr_len);
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, ip, INET_ADDRSTRLEN);
    log_info("Nueva conexión desde " + string(ip) + ":" + to_string(ntohs(addr.sin_port)));
    
    while (recv_line_raw(client_sock, line)) {
        vector<string> parts;
        string cur;
        for (char ch : line) {
            if (ch == SEP) { parts.push_back(cur); cur.clear(); }
            else cur.push_back(ch);
        }
        if (!cur.empty()) parts.push_back(cur);
        if (parts.empty()) continue;
        string cmd = parts[0];

        if (cmd == "JOIN") {
            string name = (parts.size() > 1 ? parts[1] : string("user"));
            myId = G->add_player(client_sock, name);
            myName = name;
            
            {
                lock_guard<mutex> lk(sock_mtx);
                id_to_sock[myId] = client_sock;
            }
            
            log_event("Jugador '" + name + "' conectado (ID: " + to_string(myId) + ")");
            
            send_line_raw(client_sock, make_msg2("WELCOME", to_string(myId)));
            
            // Pequeño delay antes de enviar LOBBY a TODOS
            this_thread::sleep_for(chrono::milliseconds(100));
            broadcast_msg(make_msg2("LOBBY", G->build_player_list_csv()));
            
            // Si el juego ya está corriendo, enviar estado actual
            if (G->is_running()) {
                this_thread::sleep_for(chrono::milliseconds(100));
                auto cfg = G->get_config();
                send_line_raw(client_sock, make_msg3("START", to_string(cfg.boardSize), to_string(cfg.diceSides)));
                this_thread::sleep_for(chrono::milliseconds(100));
                broadcast_scores_to_all();
                this_thread::sleep_for(chrono::milliseconds(100));
                handle_turn_notification();
            }
            
            continue;
        }

        if (cmd == "READY") {
            if (parts.size() < 2) { 
                send_line_raw(client_sock, make_msg2("ERROR","Falta nombre del equipo")); 
                log_error("Jugador " + to_string(myId) + " intentó marcarse listo sin equipo");
                continue; 
            }
            string teamName = parts[1];
            if (myId == -1) { 
                send_line_raw(client_sock, make_msg2("ERROR","No te has unido al juego")); 
                continue; 
            }
            
            auto res = G->player_ready(myId, teamName);
            if (res != GameResult::OK) {
                send_line_raw(client_sock, make_msg2("ERROR","No puedes cambiar de equipo ahora"));
                continue;
            }
            {
                lock_guard<mutex> lk(sock_mtx);
                id_to_team[myId] = teamName;
                myTeam = teamName;
            }
            
            log_event("Jugador '" + myName + "' (ID: " + to_string(myId) + ") se unió al equipo '" + teamName + "'");
            
            // Enviar LOBBY actualizado a TODOS
            this_thread::sleep_for(chrono::milliseconds(100));
            broadcast_msg(make_msg2("LOBBY", G->build_player_list_csv()));
            
            // Intentar iniciar el juego
            bool wasRunning = G->is_running();
            G->start_game_if_ready();
            
            if (G->is_running() && !wasRunning) {
                log_game("¡JUEGO INICIADO!");
                auto cfg = G->get_config();
                log_game("Meta: " + to_string(cfg.boardSize) + " puntos | Dado: " + to_string(cfg.diceSides) + " caras");
                
                // Enviar mensajes de inicio con delays
                this_thread::sleep_for(chrono::milliseconds(150));
                broadcast_msg(make_msg3("START", to_string(cfg.boardSize), to_string(cfg.diceSides)));
                
                // IMPORTANTE: Enviar puntuaciones iniciales
                this_thread::sleep_for(chrono::milliseconds(150));
                broadcast_scores_to_all();
                
                this_thread::sleep_for(chrono::milliseconds(150));
                broadcast_board_update();
                
                this_thread::sleep_for(chrono::milliseconds(150));
                handle_turn_notification();
            }
            continue;
        }

        if (cmd == "ROLL") {
            if (myId == -1) { 
                send_line_raw(client_sock, make_msg2("ERROR","No te has unido al juego")); 
                continue; 
            }
            
            log_game("Jugador '" + myName + "' lanza el dado...");
            broadcast_msg(make_msg("ROLLING"));
            this_thread::sleep_for(chrono::milliseconds(800));
            
            auto pr = G->handle_roll(myId);
            
            if (pr.first == GameResult::NOT_RUNNING) {
                send_line_raw(client_sock, make_msg2("ERROR","El juego no está en ejecución"));
                log_error("Intento de tirada cuando el juego no está corriendo");
                continue;
            } else if (pr.first == GameResult::NOT_YOUR_TURN) {
                send_line_raw(client_sock, make_msg2("ERROR","No es tu turno"));
                log_error("Jugador '" + myName + "' intentó jugar fuera de turno");
                continue;
            } else if (pr.first == GameResult::PLAYER_DISCONNECTED) {
                send_line_raw(client_sock, make_msg2("ERROR","Jugador desconectado"));
                continue;
            } else if (pr.first == GameResult::OK) {
                string playerName = G->get_player_name(myId);
                string teamName = G->get_player_team(myId);
                int diceValue = pr.second;
                
                log_game("Jugador '" + playerName + "' sacó " + to_string(diceValue) + " puntos");
                
                // Enviar resultados con delays
                send_to_id(myId, make_msg2("ROLL_RESULT", "¡Sacaste " + to_string(diceValue) + " puntos!"));
                this_thread::sleep_for(chrono::milliseconds(100));
                
                send_to_team_except(teamName, myId, 
                    make_msg2("ROLL_RESULT", playerName + " sacó " + to_string(diceValue) + " puntos"));
                this_thread::sleep_for(chrono::milliseconds(100));
                
                send_to_other_teams(teamName, 
                    make_msg2("ROLL_RESULT", "Equipo " + teamName + " sacó " + to_string(diceValue) + " puntos"));
                
                // Enviar puntuaciones actualizadas
                this_thread::sleep_for(chrono::milliseconds(300));
                broadcast_scores_to_all();
                broadcast_board_update();
                
                // Verificar fin del juego
                if (!G->is_running()) {
                    string winner = G->get_winner();
                    log_game("¡JUEGO TERMINADO! Ganador: Equipo " + winner);
                    
                    this_thread::sleep_for(chrono::milliseconds(500));
                    broadcast_msg(make_msg2("END", "¡Equipo " + winner + " gana!"));
                    G->reset_after_game();
                    
                    this_thread::sleep_for(chrono::milliseconds(100));
                    broadcast_msg(make_msg2("LOBBY", G->build_player_list_csv()));
                } else {
                    // Siguiente turno
                    this_thread::sleep_for(chrono::milliseconds(300));
                    handle_turn_notification();
                }
            }
            continue;
        }

        if (cmd == "QUIT") {
            log_info("Jugador '" + myName + "' (ID: " + to_string(myId) + ") se desconectó voluntariamente");
            break;
        }

        send_line_raw(client_sock, make_msg2("ERROR","Comando desconocido"));
        log_error("Comando desconocido recibido de jugador " + to_string(myId) + ": " + cmd);
    }

    // Manejo de desconexión
    if (myId != -1) {
        log_info("Jugador '" + myName + "' (ID: " + to_string(myId) + ") desconectado");
        
        G->mark_player_disconnected(myId);
        {
            lock_guard<mutex> lk(sock_mtx);
            id_to_sock.erase(myId);
            id_to_team.erase(myId);
        }
        
        if (G->is_running()) {
            string playerName = G->get_player_name(myId);
            string teamName = G->get_player_team(myId);
            
            log_game("Jugador '" + playerName + "' del equipo " + teamName + " abandonó la partida");
            broadcast_msg(make_msg2("PLAYER_LEFT", playerName + "|" + teamName));
            
            this_thread::sleep_for(chrono::milliseconds(100));
            broadcast_scores_to_all();
            
            // Si era su turno, avanzar
            int currentPlayer = G->current_player_id();
            if (currentPlayer == myId) {
                log_game("Era el turno del jugador desconectado, avanzando...");
                auto pr = G->handle_roll(myId);
                this_thread::sleep_for(chrono::milliseconds(100));
                handle_turn_notification();
            }
        }
    }
    
    broadcast_msg(make_msg2("LOBBY", G->build_player_list_csv()));
    close(client_sock);
}

int main(int argc, char** argv) {
    load_env_variables();
    
    ServerConfig cfg;
    const char* v;
    if ((v = getenv("GAME_BOARD_SIZE"))) cfg.boardSize = stoi(v);
    if ((v = getenv("DICE_SIDES"))) cfg.diceSides = stoi(v);
    if ((v = getenv("MIN_TEAMS"))) cfg.minTeams = stoi(v);
    if ((v = getenv("MIN_PLAYERS_PER_TEAM"))) cfg.minPlayersPerTeam = stoi(v);
    if ((v = getenv("MAX_TEAMS"))) cfg.maxTeams = stoi(v);
    if ((v = getenv("GAME_PORT"))) cfg.port = stoi(v);

    cerr << endl;
    cerr << "╔══════════════════════════════════════╗" << endl;
    cerr << "║      🎮 SERVIDOR DE JUEGO 🎮         ║" << endl;
    cerr << "╠══════════════════════════════════════╣" << endl;
    cerr << "║ Tablero:  " << setw(3) << cfg.boardSize << " posiciones" << setw(15) << "║" << endl;
    cerr << "║ Dado:     " << setw(2) << cfg.diceSides << " caras" << setw(22) << "║" << endl;
    cerr << "║ Puerto:   " << setw(4) << cfg.port << setw(25) << "║" << endl;
    cerr << "║ Min equipos: " << cfg.minTeams << setw(23) << "║" << endl;
    cerr << "║ Min jugadores/equipo: " << cfg.minPlayersPerTeam << setw(13) << "║" << endl;
    cerr << "╚══════════════════════════════════════╝" << endl;
    cerr << endl;

    G = new Game(cfg);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }
    
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(cfg.port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed"); return 1;
    }
    
    if (listen(server_fd, 16) < 0) { perror("listen"); return 1; }
    
    log_info("Servidor iniciado. Esperando conexiones...");
    cerr << endl;

    while (true) {
        sockaddr_in client_addr;
        socklen_t addrlen = sizeof(client_addr);
        int new_socket = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);
        if (new_socket < 0) { 
            perror("accept"); 
            continue; 
        }
        
        thread t(handle_client, new_socket);
        t.detach();
    }

    delete G;
    close(server_fd);
    return 0;
}
