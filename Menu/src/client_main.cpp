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
string myTeam = "";

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
    cout << "\033[2J\033[1;1H";
}

void show_header(const string& username) {
    cout << "╔══════════════════════════════════════╗" << endl;
    cout << "║           JUEGO MULTIPLAYER          ║" << endl;
    cout << "╠══════════════════════════════════════╣" << endl;
    cout << "║ Jugador: " << username;
    if (!myTeam.empty()) {
        cout << " [" << myTeam << "]";
    }
    
    int total_length = username.length();
    if (!myTeam.empty()) {
        total_length += myTeam.length() + 3;
    }
    int padding = 34 - total_length;
    
    cout << setw(padding) << "║" << endl;
    cout << "╚══════════════════════════════════════╝" << endl;
}

void reader_thread_func(int sock, const string& username) {
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
        
        lock_guard<mutex> lk(cout_m);
        clear_screen();
        show_header(username);
        
        if (type == "YOURTURN") {
            myTurn = true;
            string message = parts.size()>1?parts[1]:"";
            cout << endl << " " << message << endl;
        } 
        else if (type == "ROLLING") {
            cout << endl << " Lanzando el dado";
            for (int i = 0; i < 3; i++) {
                cout << ".";
                cout.flush();
                this_thread::sleep_for(chrono::milliseconds(300));
            }
            cout << endl;
        } 
        else if (type == "ROLL_RESULT") {
            string result = parts.size()>1?parts[1]:"";
            cout << endl << " " << result << endl;
        } 
        else if (type == "SCORES") {
            if (parts.size() > 1) {
                cout << endl << parts[1] << endl;
            }
        } 
        else if (type == "START") {
            cout << endl << "¡JUEGO INICIADO!" << endl;
            if (parts.size() > 1) {
                cout << "Meta: " << parts[1] << " puntos" << endl;
            }
            if (parts.size() > 2) {
                cout << "Dado: " << parts[2] << " caras" << endl;
            }
        } 
        else if (type == "END") {
            cout << endl << "¡JUEGO TERMINADO!" << endl;
            cout << "Ganador: " << (parts.size()>1?parts[1]:"") << endl;
            running = false;
            break;
        } 
        else if (type == "LOBBY") {
            cout << endl << " Sala: " << (parts.size()>1?parts[1]:"") << endl;
        } 
        else if (type == "WELCOME") {
            cout << endl << " Conectado al servidor" << endl;
        } 
        else if (type == "TURN_INFO") {
            string turnInfo = parts.size()>1?parts[1]:"";
            cout << endl << "" << turnInfo << endl;
        } 
        else if (type == "PLAYER_LEFT") {
            string info = parts.size()>1?parts[1]:"";
            size_t sep_pos = info.find('|');
            if (sep_pos != string::npos) {
                string playerName = info.substr(0, sep_pos);
                string teamName = info.substr(sep_pos + 1);
                cout << endl << " " << playerName << " del equipo " << teamName << " se desconectó" << endl;
            }
        } 
        else if (type == "ERROR") {
            cout << endl << " Error: " << (parts.size()>1?parts[1]:"") << endl;
        } 
        else {
            cout << "[MSG] " << line << endl;
        }
        
        if (myTurn) {
            cout << endl << ">>> Escribe 'r' para lanzar el dado > ";
        } else {
            cout << endl << ">>> Comandos: 'ready <equipo>' | 'quit' > ";
        }
        cout.flush();
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
    string username = argv[3];

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
    thread reader(reader_thread_func, sock, username);
    send_line(sock, make_msg2("JOIN", username));

    while (running) {
        string line;
        if (!getline(cin, line)) break;

        auto trim = [](string &s){
            while (!s.empty() && isspace((unsigned char)s.front())) s.erase(s.begin());
            while (!s.empty() && isspace((unsigned char)s.back())) s.pop_back();
        };
        trim(line);
        if (line.empty()) continue;

        if (myTurn) {
            char c = tolower((unsigned char)line[0]);
            if (c == 'r') {
                if (!send_line(sock, make_msg("ROLL"))) {
                    cerr << "Error enviando ROLL" << endl;
                    running = false;
                    break;
                }
                myTurn = false;
            } else if (c == 'q') {
                send_line(sock, make_msg("QUIT"));
                myTurn = false;
            } else {
                lock_guard<mutex> lk(cout_m);
                cout << "Comando no reconocido. Usa 'r' para lanzar" << endl;
            }
        } else {
            stringstream ss(line);
            string cmd; 
            ss >> cmd;
            
            if (cmd == "ready") {
                string team; 
                ss >> team;
                if (team.empty()) team = "Equipo" + to_string(rand() % 1000);
                myTeam = team;
                send_line(sock, make_msg2("READY", team));
            } else if (cmd == "quit") {
                send_line(sock, make_msg("QUIT"));
                running = false;
                break;
            } else {
                lock_guard<mutex> lk(cout_m);
                cout << "Comando no reconocido. Usa 'ready <equipo>' o 'quit'" << endl;
            }
        }
    }

    close(sock);
    if (reader.joinable()) reader.join();
    cout << "Cliente finalizado" << endl;
    return 0;
}