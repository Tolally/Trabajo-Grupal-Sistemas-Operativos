#include "../include/game.h"
#include "../include/game_protocol.h"
#include <algorithm>
#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>
#include <unordered_set>

using namespace std;

Game::Game(const ServerConfig& cfg_) : cfg(cfg_) {
    auto seed = (unsigned)chrono::high_resolution_clock::now().time_since_epoch().count();
    rng.seed(seed);
}

Game::~Game(){}

int Game::add_player(int sock, const std::string& name) {
    int id = nextPlayerId++;
    PlayerInfo p; 
    p.id = id; 
    p.sock = sock; 
    p.name = name;
    p.connected = true;
    players[id] = p;
    return id;
}

void Game::remove_player(int id) {
    auto it = players.find(id);
    if (it != players.end()) {
        it->second.connected = false;
    }
}

GameResult Game::player_ready(int id, const std::string& teamName) {
    auto it = players.find(id);
    if (it == players.end()) return GameResult::INVALID_ARG;
    
    it->second.ready = true;
    it->second.team = teamName;
    
    auto &t = teams[teamName];
    t.name = teamName;
    if (find(t.members.begin(), t.members.end(), id) == t.members.end()) {
        t.members.push_back(id);
    }
    
    return GameResult::OK;
}

void Game::assign_team_symbols() {
    int symbol_idx = 0;
    for (auto &kv : teams) {
        if (symbol_idx < (int)team_symbols.size()) {
            kv.second.symbol = team_symbols[symbol_idx++];
            team_to_symbol[kv.first] = kv.second.symbol;
        } else {
            kv.second.symbol = '?';
            team_to_symbol[kv.first] = '?';
        }
    }
}

void Game::build_play_order_round_robin() {
    play_order.clear();
    vector<vector<int>> lists;
    for (auto &kv : teams) {
        lists.push_back(kv.second.members);
    }
    
    size_t maxlen = 0;
    for (auto &l : lists) if (l.size() > maxlen) maxlen = l.size();
    for (size_t i = 0; i < maxlen; ++i) {
        for (auto &l : lists) {
            if (i < l.size()) play_order.push_back(l[i]);
        }
    }
    current_turn_idx = 0;
}

bool Game::check_start_conditions() {
    if (running) return false;

    int teams_ok = 0;
    for (auto &kv : teams) {
        if ((int)kv.second.members.size() >= cfg.minPlayersPerTeam) ++teams_ok;
    }
    if (teams_ok < cfg.minTeams) return false;

    for (auto &kv : players) {
        if (!kv.second.ready) return false;
    }

    return true;
}

bool Game::is_team_active(const std::string& teamName) {
    auto it = teams.find(teamName);
    if (it == teams.end()) return false;
    
    for (int playerId : it->second.members) {
        auto playerIt = players.find(playerId);
        if (playerIt != players.end() && playerIt->second.connected) {
            return true;
        }
    }
    return false;
}

void Game::start_game_if_ready() {
    if (running) return;
    if (!check_start_conditions()) return;
    
    for (auto &kv : teams) {
        kv.second.score = 0;
        kv.second.active = true;
    }
    
    assign_team_symbols();
    build_play_order_round_robin();
    running = true;
}

void Game::advance_turn() {
    int start_idx = current_turn_idx;
    
    do {
        current_turn_idx = (current_turn_idx + 1) % play_order.size();
        
        int currentPlayerId = play_order[current_turn_idx];
        auto playerIt = players.find(currentPlayerId);
        if (playerIt != players.end() && playerIt->second.connected) {
            if (is_team_active(playerIt->second.team)) {
                return;
            }
        }
        
        if (current_turn_idx == start_idx) {
            break;
        }
    } while (true);
}

pair<GameResult,int> Game::handle_roll(int id) {
    if (!running) return {GameResult::NOT_RUNNING, 0};
    if (play_order.empty()) return {GameResult::ERROR, 0};
    
    int expectedId = play_order[current_turn_idx];
    if (id != expectedId) return {GameResult::NOT_YOUR_TURN, 0};
    
    auto playerIt = players.find(id);
    if (playerIt == players.end() || !playerIt->second.connected) {
        return {GameResult::PLAYER_DISCONNECTED, 0};
    }
    
    uniform_int_distribution<int> dist(1, cfg.diceSides);
    int val = dist(rng);
    
    string team = players[id].team;
    if (teams.find(team) == teams.end()) return {GameResult::ERROR, 0};
    
    teams[team].score += val;
    
    bool game_ended = false;
    string winning_team;
    for (auto &kv : teams) {
        if (kv.second.score >= cfg.boardSize && is_team_active(kv.first)) {
            game_ended = true;
            winning_team = kv.first;
            break;
        }
    }
    
    int active_teams = 0;
    string last_active_team;
    for (auto &kv : teams) {
        if (is_team_active(kv.first)) {
            active_teams++;
            last_active_team = kv.first;
        }
    }
    
    if (active_teams <= 1 && !last_active_team.empty()) {
        game_ended = true;
        winning_team = last_active_team;
    }
    
    if (game_ended) {
        running = false;
        return {GameResult::OK, val};
    }
    
    advance_turn();
    return {GameResult::OK, val};
}

string Game::build_player_list_csv() {
    stringstream ss;
    bool first = true;
    for (auto &kv : players) {
        if (!first) ss << ",";
        ss << kv.second.name << "(" << (kv.second.ready ? "Listo" : "Esperando") << ")";
        if (!kv.second.team.empty()) ss << "[" << kv.second.team << "]";
        if (!kv.second.connected) ss << "{DESCONECTADO}";
        first = false;
    }
    return ss.str();
}

string Game::build_teams_status() {
    stringstream ss;
    bool first = true;
    for (auto &kv : teams) {
        if (!first) ss << ";";
        ss << kv.first << "," << kv.second.score << "," << (is_team_active(kv.first) ? "1" : "0");
        first = false;
    }
    return ss.str();
}

string Game::build_scores_display() {
    stringstream ss;
    ss << "\n🏆 PUNTUACIONES ACTUALES 🏆" << endl;
    ss << "Meta: " << cfg.boardSize << " puntos" << endl << endl;
    
    for (auto &kv : teams) {
        string status = is_team_active(kv.first) ? "🏃 ACTIVO" : "💀 ELIMINADO";
        ss << "Equipo " << kv.first << ": " << kv.second.score << "/" << cfg.boardSize << " puntos - " << status << endl;
    }
    
    int currentId = current_player_id();
    if (currentId != -1 && running) {
        auto playerIt = players.find(currentId);
        if (playerIt != players.end()) {
            ss << endl << "📢 Turno actual: " << playerIt->second.name << " (Equipo " << playerIt->second.team << ")" << endl;
        }
    }
    
    return ss.str();
}

string Game::build_board_display() {
    stringstream ss;
    ss << "┌─────────────────────────────────────────────────┐" << endl;
    ss << "│              CARRERA DE EQUIPOS                 │" << endl;
    ss << "├─────────────────────────────────────────────────┤" << endl;
    ss << "│ Meta: " << setw(3) << cfg.boardSize << " posiciones" << setw(32) << "│" << endl;
    
    for (auto &kv : teams) {
        string status = is_team_active(kv.first) ? "ACTIVO " : "ELIMINADO";
        int progress = min(kv.second.score, cfg.boardSize);
        
        ss << "├─────────────────────────────────────────────────┤" << endl;
        ss << "│ Equipo " << kv.first << " [" << kv.second.symbol << "] " 
           << status << " " << setw(3) << progress << "/" << setw(3) << cfg.boardSize << " │" << endl;
        
        int bar_width = 30;
        int pos = (progress * bar_width) / cfg.boardSize;
        ss << "│ [";
        for (int i = 0; i < bar_width; i++) {
            if (i < pos) ss << kv.second.symbol;
            else if (i == pos) ss << "○";
            else ss << "─";
        }
        ss << "] │" << endl;
        
        bool first_player = true;
        for (int playerId : kv.second.members) {
            auto playerIt = players.find(playerId);
            if (playerIt != players.end()) {
                if (first_player) {
                    ss << "│ ";
                    first_player = false;
                } else {
                    ss << "  ";
                }
                ss << (playerIt->second.connected ? "●" : "○") << " " << playerIt->second.name;
                if (playerId == current_player_id()) ss << " ←";
            }
        }
        if (!first_player) ss << endl;
    }
    ss << "└─────────────────────────────────────────────────┘" << endl;
    
    return ss.str();
}

int Game::current_player_id() {
    if (!running || play_order.empty()) return -1;
    return play_order[current_turn_idx];
}

string Game::current_player_name() {
    int id = current_player_id();
    if (id == -1) return "";
    auto it = players.find(id);
    if (it != players.end()) return it->second.name;
    return "";
}

string Game::current_player_team() {
    int id = current_player_id();
    if (id == -1) return "";
    auto it = players.find(id);
    if (it != players.end()) return it->second.team;
    return "";
}

bool Game::is_running() { return running; }
ServerConfig Game::get_config() { return cfg; }

void Game::reset_after_game() {
    running = false;
    play_order.clear();
    current_turn_idx = 0;
    for (auto &kv : players) {
        kv.second.ready = false;
        kv.second.team.clear();
    }
    teams.clear();
    team_to_symbol.clear();
}

string Game::get_winner() {
    string winner;
    int max_score = -1;
    for (auto &kv : teams) {
        if (is_team_active(kv.first) && kv.second.score > max_score) {
            max_score = kv.second.score;
            winner = kv.first;
        }
    }
    return winner;
}

string Game::get_player_name(int id) {
    auto it = players.find(id);
    if (it != players.end()) return it->second.name;
    return "JugadorDesconocido";
}

string Game::get_player_team(int id) {
    auto it = players.find(id);
    if (it != players.end()) return it->second.team;
    return "";
}

char Game::get_team_symbol(const std::string& teamName) {
    auto it = team_to_symbol.find(teamName);
    if (it != team_to_symbol.end()) return it->second;
    return '?';
}

void Game::mark_player_disconnected(int id) {
    auto it = players.find(id);
    if (it != players.end()) {
        it->second.connected = false;
    }
}

int Game::get_team_player_count(const std::string& teamName) {
    auto it = teams.find(teamName);
    if (it == teams.end()) return 0;
    
    int count = 0;
    for (int playerId : it->second.members) {
        auto playerIt = players.find(playerId);
        if (playerIt != players.end() && playerIt->second.connected) {
            count++;
        }
    }
    return count;
}