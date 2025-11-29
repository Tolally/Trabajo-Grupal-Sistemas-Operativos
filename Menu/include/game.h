#ifndef GAME_H
#define GAME_H

#include <string>
#include <map>
#include <vector>
#include <random>
#include <unordered_set>
#include <chrono>

struct ServerConfig {
    int boardSize = 50;
    int diceSides = 6;
    int minTeams = 2;
    int minPlayersPerTeam = 2;
    int maxTeams = 4;
    int port = 8080;
    std::string statsFile;
};

enum class GameResult {
    OK,
    NOT_RUNNING,
    NOT_YOUR_TURN,
    INVALID_ARG,
    ERROR,
    PLAYER_DISCONNECTED
};

struct PlayerInfo {
    int id;
    int sock;
    std::string name;
    std::string team;
    bool ready = false;
    bool connected = true;
};

struct TeamInfo {
    std::string name;
    std::vector<int> members;
    int score = 0;
    char symbol;
    bool active = true;
};

class Game {
private:
    ServerConfig cfg;
    std::map<int, PlayerInfo> players;
    std::map<std::string, TeamInfo> teams;
    std::vector<int> play_order;
    int current_turn_idx = 0;
    int nextPlayerId = 1;
    bool running = false;
    std::mt19937 rng;
    
    // Símbolos para los equipos
    std::vector<char> team_symbols = {'*', '+', '^', '$'};
    std::map<std::string, char> team_to_symbol;

    // Estadísticas del juego
    int total_turns_played = 0;
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point end_time;

    void build_play_order_round_robin();
    bool check_start_conditions();
    void assign_team_symbols();
    void advance_turn();
    bool is_team_active(const std::string& teamName);

public:
    Game(const ServerConfig& cfg_);
    ~Game();

    int add_player(int sock, const std::string& name);
    void remove_player(int id);
    GameResult player_ready(int id, const std::string& teamName);
    void start_game_if_ready();
    std::pair<GameResult, int> handle_roll(int id);
    
    std::string build_player_list_csv();
    std::string build_teams_status();
    std::string build_turn_info();
    std::string build_board_display();
    std::string build_scores_display();
    int current_player_id();
    std::string current_player_name();
    std::string current_player_team();
    bool is_running();
    ServerConfig get_config();
    void reset_after_game();
    std::string get_winner();
    
    std::string get_player_name(int id);
    std::string get_player_team(int id);
    char get_team_symbol(const std::string& teamName);
    
    void mark_player_disconnected(int id);
    int get_next_active_player();
    int get_team_player_count(const std::string& teamName);
    
    void save_statistics_to_csv();
};

#endif
