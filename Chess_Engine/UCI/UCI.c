#include <fcntl.h>
#include <pthread.h>
#include <stdarg.h>
#include <strings.h>


#include "../Alpha_Beta/Openings.h"
#include "../Alpha_Beta/Pre_Computed_Data.h"
#include "../Alpha_Beta/Search.h"
#include "../Alpha_Beta/Transposition_Table.h"
#include "../Game_Treatment/Board.h"
#include "../Game_Treatment/Fen.h"
#include "../Game_Treatment/Log.h"
#include "../Game_Treatment/Move_Generator.h"
#include "../Game_Treatment/Moves_Treatment.h"
#include "../Game_Treatment/Settings.h"
#include "../Useful/Constants.h"
#include "../Useful/Utility.h"


///////////////////////////////////////////////////////////////////////////////
// UCI : the chess engine as a program that talks the UCI protocol (Universal Chess Interface)
// It reads commands on its standard input and answers on its standard output, which is how the tournament
// managers (Fastchess, Cute Chess) and the chess graphical interfaces make an engine play.
/**
 * A summary of the supported commands is given below:
 *
 * uci : the engine gives its name, its options, and answers "uciok"
 * isready : the engine answers "readyok"
 * setoption name <name> value <value> : to change an option (Hash, OwnBook, BookPlies, BookFile, Seed, Move Overhead)
 * ucinewgame : a new game starts, the transposition table is emptied
 * position startpos [moves ...] : to set the position from the start of the game and the moves played
 * position fen <fen> [moves ...] : to set the position from a FEN and the moves played
 * go movetime <ms> : to search during a given time
 * go depth <n> : to search until a given depth
 * go wtime <ms> btime <ms> [winc <ms>] [binc <ms>] [movestogo <n>] : to search with the clocks of the game
 * go infinite : to search until "stop"
 * stop : to stop the search, the engine gives the best move found
 * quit : to close the engine
 *
 * and three commands that aren't in the protocol, to help the tests :
 * d : to show the position
 * bench [depth] : to search a set of positions at a given depth and show the counters of the search (nodes, speed, cutoffs, ...)
 * debug on / debug off : to show (or not) the debug texts of the engine on the error output
**/
///////////////////////////////////////////////////////////////////////////////


#define ENGINE_NAME "Cleaned Chess Engine"
#define ENGINE_VERSION "1.2"
#define ENGINE_AUTHOR "tomcuel"
#define UCI_LINE_SIZE 16384
#define DEFAULT_HASH_SIZE_MB 64
#define DEFAULT_BOOK_PLIES 10
#define DEFAULT_MOVE_OVERHEAD_MS 30


// where the answers of the protocol are written (the standard output is kept for it only, the debug texts of the engine go elsewhere)
static FILE* uci_output = NULL;
// the options of the engine
static int hash_size_MB = DEFAULT_HASH_SIZE_MB;
static bool is_own_book_used = true;
static int book_plies = DEFAULT_BOOK_PLIES;
static int move_overhead_ms = DEFAULT_MOVE_OVERHEAD_MS;
// the search runs in its own thread, so the commands (stop, isready, ...) can still be read during it
static pthread_t search_thread;
static bool is_search_thread_running = false;
static bool is_search_without_limit = false; // true for "go infinite" : only the command "stop" ends the search
static int search_start_time = 0;
// the counters of the last search, kept to be shown at the end of it
static int last_depth_completed = 0;


// Function to write an answer of the protocol
static void uci_print(const char* format, ...) {

    va_list arguments;
    va_start(arguments, format);
    vfprintf(uci_output, format, arguments);
    va_end(arguments);
    fflush(uci_output);

}


// Function to send the debug texts of the engine (its printf) to the error output, or to nowhere
static void set_debug_output(bool is_enabled) {

    fflush(stdout);
    if (is_enabled) {
        dup2(STDERR_FILENO, STDOUT_FILENO);
    }
    else {
        int null_file = open("/dev/null", O_WRONLY);
        if (null_file != -1) {
            dup2(null_file, STDOUT_FILENO);
            close(null_file);
        }
    }

}


// Function to write a move in the UCI notation (ex : e2e4, e1g1 for a castle, e7e8q for a promotion)
static void get_move_name(move move_value, char* name) {

    if (is_move_null(move_value)) {
        strcpy(name, "0000");
        return;
    }
    int from = get_from_square(move_value);
    int to = get_to_square(move_value);
    int flag = get_flag(move_value);
    name[0] = 'a' + get_file(from);
    name[1] = '1' + get_rank(from);
    name[2] = 'a' + get_file(to);
    name[3] = '1' + get_rank(to);
    name[4] = '\0';
    name[5] = '\0';
    if (flag == Promote_To_Queen_Flag) name[4] = 'q';
    if (flag == Promote_To_Rook_Flag) name[4] = 'r';
    if (flag == Promote_To_Bishop_Flag) name[4] = 'b';
    if (flag == Promote_To_Knight_Flag) name[4] = 'n';

}


// Function to find the legal move of the current position that corresponds to a move in the UCI notation (0 if there is none)
static move get_move_from_name(const char* name) {

    // the opponent can promote to another piece than a queen, so all the promotions are needed here
    are_under_promotions_generated = true;
    int nb_moves = 0;
    move* valid_moves = generate_all_possible_moves(&nb_moves);
    are_under_promotions_generated = false;

    move move_found = 0;
    for (int i = 0; i < nb_moves; i++) {
        char valid_move_name[6];
        get_move_name(valid_moves[i], valid_move_name);
        if (strcmp(valid_move_name, name) == 0) {
            move_found = valid_moves[i];
            break;
        }
    }
    free(valid_moves);
    return move_found;

}


// Function to get the part of the transposition table that is used, in per thousand (looking at its first thousand entries)
static int get_hashfull() {

    int nb_entries_looked = min(1000, transposition_tables->size);
    int nb_entries_used = 0;
    for (int i = 0; i < nb_entries_looked; i++) {
        if (transposition_tables->entries[i].zobrist_key != 0) {
            nb_entries_used++;
        }
    }
    return (nb_entries_looked > 0) ? (nb_entries_used * 1000) / nb_entries_looked : 0;

}


// Function called by the search each time a depth is completed, to give the information of the search
static void print_search_info(int depth, int score, move best_move_found) {

    last_depth_completed = depth;
    int time_taken = get_time_ms() - search_start_time;
    long long nodes = search_statistics.nodes;
    long long nodes_per_second = (time_taken > 0) ? (nodes * 1000) / time_taken : nodes * 1000;
    char name[6];
    get_move_name(best_move_found, name);

    // a score close to the mate score is given as a number of moves before the mate
    char score_text[32];
    if (abs(score) > IMMEDIATE_MATE_SCORE - 1000) {
        int nb_plies_to_mate = IMMEDIATE_MATE_SCORE - abs(score);
        int nb_moves_to_mate = (nb_plies_to_mate + 1) / 2;
        if (nb_moves_to_mate < 1) {
            nb_moves_to_mate = 1;
        }
        snprintf(score_text, sizeof(score_text), "mate %d", (score > 0) ? nb_moves_to_mate : -nb_moves_to_mate);
    }
    else {
        snprintf(score_text, sizeof(score_text), "cp %d", score);
    }

    uci_print("info depth %d score %s nodes %lld nps %lld hashfull %d time %d pv %s\n", depth, score_text, nodes, nodes_per_second, get_hashfull(), time_taken, name);

}


// Function to show the counters of the last search (it isn't a part of the protocol, so it's given as a free text)
static void print_search_statistics() {

    long long beta_cutoffs = search_statistics.beta_cutoffs;
    long long first_move_percentage = (beta_cutoffs > 0) ? (search_statistics.first_move_beta_cutoffs * 100) / beta_cutoffs : 0;
    uci_print("info string stats depth %d nodes %lld qnodes %lld tthits %lld cutoffs %lld firstmovecutoffs %lld%%\n", last_depth_completed, search_statistics.nodes, search_statistics.quiescence_nodes, search_statistics.transposition_hits, beta_cutoffs, first_move_percentage);

}


// Function to look for the best move of the current position (in the opening book first, then with the search)
static move search_best_move() {

    last_depth_completed = 0;
    search_start_time = get_time_ms();

    // looking in the opening book
    move best_move_found = get_move_from_book();
    if (!is_move_null(best_move_found)) {
        char name[6];
        get_move_name(best_move_found, name);
        // the programs that run the games expect an information line with a score before the move, even without a search
        uci_print("info depth 0 score cp 0 nodes 0 nps 0 time 0 pv %s\n", name);
        uci_print("info string book move %s\n", name);
        return best_move_found;
    }

    // running the search
    int selected_score = 0;
    int depth_reached = 0;
    run_iterative_deepening(&best_move_found, &selected_score, &depth_reached);
    print_search_statistics();
    return best_move_found;

}


// Function run by the search thread : it searches the best move and gives it
static void* run_search_thread(void* unused) {

    (void)unused;
    move best_move_found = search_best_move();
    char name[6];
    get_move_name(best_move_found, name);
    uci_print("bestmove %s\n", name);
    return NULL;

}


// Function to stop the search if there is one, and to wait for its end
static void stop_search() {

    if (is_search_thread_running) {
        is_search_stop_requested = true;
        pthread_join(search_thread, NULL);
        is_search_thread_running = false;
    }
    is_search_stop_requested = false;

}


// Function to get the value following a word in a command (ex : the 1000 of "go movetime 1000"), default_value if the word isn't there
static int get_int_after_word(const char* command, const char* word, int default_value) {

    size_t word_size = strlen(word);
    const char* position = command;
    while ((position = strstr(position, word)) != NULL) {
        // the word must be a whole word of the command
        bool is_start_of_word = (position == command || position[-1] == ' ');
        bool is_end_of_word = (position[word_size] == ' ');
        if (is_start_of_word && is_end_of_word) {
            return atoi(position + word_size + 1);
        }
        position += word_size;
    }
    return default_value;

}


// Function to know if a word is in a command
static bool is_word_in_command(const char* command, const char* word) {

    size_t word_size = strlen(word);
    const char* position = command;
    while ((position = strstr(position, word)) != NULL) {
        bool is_start_of_word = (position == command || position[-1] == ' ');
        bool is_end_of_word = (position[word_size] == ' ' || position[word_size] == '\0');
        if (is_start_of_word && is_end_of_word) {
            return true;
        }
        position += word_size;
    }
    return false;

}


// Function to treat the command "position" : position [startpos | fen <fen>] [moves <move> <move> ...]
static void treat_position_command(char* command) {

    // separating the moves from the rest of the command
    char* moves = strstr(command, " moves");
    if (moves != NULL) {
        *moves = '\0';
        moves += strlen(" moves");
    }

    // setting the position
    char* fen = strstr(command, "fen ");
    if (fen != NULL) {
        if (!set_position_from_fen(fen + strlen("fen "))) {
            uci_print("info string error : the FEN isn't valid, the start position is used\n");
        }
    }
    else {
        set_start_position();
    }

    // playing the moves
    if (moves != NULL) {
        char* name = strtok(moves, " ");
        while (name != NULL) {
            move move_value = get_move_from_name(name);
            if (is_move_null(move_value)) {
                uci_print("info string error : the move %s isn't legal, the following moves are ignored\n", name);
                break;
            }
            make_move_AI(move_value);
            name = strtok(NULL, " ");
        }
    }

}


// Function to choose the time of the search with the clocks of the game (in milliseconds)
static int choose_think_time_with_clocks(int time_remaining, int increment, int moves_to_go) {

    // a part of the time remaining, plus most of the increment
    int nb_moves_expected = (moves_to_go > 0) ? moves_to_go + 1 : 30;
    int think_time = time_remaining / nb_moves_expected + (increment * 3) / 4;
    // never more than half of the time remaining
    think_time = min(think_time, time_remaining / 2);
    // keeping some time for the communication with the program that runs the game
    think_time -= move_overhead_ms;
    return max(think_time, 5);

}


// Function to treat the command "go"
static void treat_go_command(const char* command) {

    int movetime = get_int_after_word(command, "movetime", 0);
    int depth = get_int_after_word(command, "depth", 0);
    bool is_white_playing = (game_settings->color_of_player_that_is_playing == WHITE);
    int time_remaining = get_int_after_word(command, is_white_playing ? "wtime" : "btime", 0);
    int increment = get_int_after_word(command, is_white_playing ? "winc" : "binc", 0);
    int moves_to_go = get_int_after_word(command, "movestogo", 0);

    // the limits of the search
    forced_max_depth = (depth > 0) ? depth : 0;
    is_search_without_limit = (movetime <= 0 && time_remaining <= 0 && depth <= 0);
    if (movetime > 0) {
        forced_think_time_in_ms = max(movetime - move_overhead_ms, 5);
    }
    else if (time_remaining > 0) {
        forced_think_time_in_ms = choose_think_time_with_clocks(time_remaining, increment, moves_to_go);
    }
    else {
        // "go depth" or "go infinite" : no limit of time, the search stops with the depth or with the command "stop"
        forced_think_time_in_ms = INT_MAX;
    }

    // running the search in its own thread
    is_search_stop_requested = false;
    if (pthread_create(&search_thread, NULL, run_search_thread, NULL) == 0) {
        is_search_thread_running = true;
    }
    else {
        uci_print("info string error : the search thread couldn't be created\n");
        uci_print("bestmove 0000\n");
    }

}


// Function to treat the command "setoption" : setoption name <name> [value <value>]
static void treat_setoption_command(const char* command) {

    const char* name = strstr(command, "name ");
    if (name == NULL) {
        return;
    }
    name += strlen("name ");
    const char* value = strstr(command, " value ");
    const char* value_text = (value != NULL) ? value + strlen(" value ") : "";
    size_t name_size = (value != NULL) ? (size_t)(value - name) : strlen(name);

    if (name_size == strlen("Hash") && strncasecmp(name, "Hash", name_size) == 0) {
        hash_size_MB = int_clamp(atoi(value_text), 1, 1024);
        free_transposition_tables();
        initialize_transposition_tables(hash_size_MB);
    }
    else if (name_size == strlen("OwnBook") && strncasecmp(name, "OwnBook", name_size) == 0) {
        is_own_book_used = (strncasecmp(value_text, "true", 4) == 0);
        set_opening_book_usage(is_own_book_used, book_plies);
    }
    else if (name_size == strlen("BookPlies") && strncasecmp(name, "BookPlies", name_size) == 0) {
        book_plies = int_clamp(atoi(value_text), 0, NUMBER_OF_OPENING_MOVES);
        set_opening_book_usage(is_own_book_used, book_plies);
    }
    else if (name_size == strlen("BookFile") && strncasecmp(name, "BookFile", name_size) == 0) {
        set_opening_book_path(value_text);
        // the book is read by playing its moves on the board from the start position, so the position has to be set again after it
        set_start_position();
        load_opening_book();
        set_start_position();
        uci_print("info string opening book : %d lines loaded\n", get_number_of_opening_book_lines());
    }
    else if (name_size == strlen("Seed") && strncasecmp(name, "Seed", name_size) == 0) {
        // the seed of the random choices in the opening book, to be able to play the same games again
        set_fixed_seed((unsigned int)strtoul(value_text, NULL, 10));
    }
    else if (name_size == strlen("Move Overhead") && strncasecmp(name, "Move Overhead", name_size) == 0) {
        move_overhead_ms = int_clamp(atoi(value_text), 0, 5000);
    }
    // the other options are ignored

}


// Function to show the position (the white pieces are in capital letters)
static void print_position() {

    const char piece_letters[] = "PNBRQK.pnbrqk.";
    for (int rank = 7; rank >= 0; rank--) {
        char line[32] = "";
        for (int file = 0; file < 8; file++) {
            int piece_type = get_type_of_piece(bitboards, get_index(rank, file));
            char square[3] = {(piece_type == NOTHING) ? '.' : piece_letters[piece_type], ' ', '\0'};
            strcat(line, square);
        }
        uci_print("info string %d  %s\n", rank + 1, line);
    }
    uci_print("info string    a b c d e f g h\n");
    uci_print("info string %s to move, %d moves in the log\n", (game_settings->color_of_player_that_is_playing == WHITE) ? "white" : "black", chess_log->actual_size);

}


// Function to treat the command "bench" : it searches some positions at a given depth, without the opening book,
// and shows the counters of the search. The number of nodes only depends on the code of the engine (not on the speed
// of the computer), so it's a way to compare two versions of the search.
static void treat_bench_command(const char* command) {

    static const char* bench_positions[] = {
        START_POSITION_FEN,
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
        "r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4",
        "rnbqkb1r/pp2pppp/3p1n2/8/3NP3/2N5/PPP2PPP/R1BQKB1R b KQkq - 2 5",
        "r2q1rk1/ppp2ppp/2npbn2/2b1p3/2B1P3/2NP1N2/PPP2PPP/R1BQ1RK1 w - - 0 8",
        "2r3k1/pp3ppp/2n1b3/3p4/3P4/2N1PN2/PP3PPP/2R3K1 w - - 0 20",
        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
        "8/5pk1/6p1/8/3K4/8/5PPP/8 w - - 0 40",
        "4k3/8/8/8/8/8/4P3/4K3 w - - 0 1",
    };
    int nb_positions = sizeof(bench_positions) / sizeof(bench_positions[0]);
    int depth = int_clamp(get_int_after_word(command, "bench", 6), 1, MAX_DEPTH_ITERATIVE_DEEPENING - 1);

    // the search is limited by the depth only, and the opening book isn't used
    set_opening_book_usage(false, book_plies);
    forced_max_depth = depth;
    forced_think_time_in_ms = INT_MAX;
    void (*callback)(int, int, move) = search_iteration_callback;
    search_iteration_callback = NULL;

    Search_Statistics total = {0, 0, 0, 0, 0};
    int time_at_start = get_time_ms();
    for (int i = 0; i < nb_positions; i++) {
        // each position is searched with an empty transposition table, so the result doesn't depend on the order
        reset_transposition_tables();
        set_position_from_fen(bench_positions[i]);
        move best_move_found = 0;
        int selected_score = 0;
        int depth_reached = 0;
        int time_before = get_time_ms();
        run_iterative_deepening(&best_move_found, &selected_score, &depth_reached);
        char name[6];
        get_move_name(best_move_found, name);
        uci_print("info string bench position %d bestmove %s score %d nodes %lld time %d\n", i + 1, name, selected_score, search_statistics.nodes, get_time_ms() - time_before);
        total.nodes += search_statistics.nodes;
        total.quiescence_nodes += search_statistics.quiescence_nodes;
        total.transposition_hits += search_statistics.transposition_hits;
        total.beta_cutoffs += search_statistics.beta_cutoffs;
        total.first_move_beta_cutoffs += search_statistics.first_move_beta_cutoffs;
    }
    int time_taken = get_time_ms() - time_at_start;
    long long nodes_per_second = (time_taken > 0) ? (total.nodes * 1000) / time_taken : 0;
    long long first_move_percentage = (total.beta_cutoffs > 0) ? (total.first_move_beta_cutoffs * 100) / total.beta_cutoffs : 0;
    uci_print("bench depth %d positions %d nodes %lld qnodes %lld tthits %lld cutoffs %lld firstmovecutoffs %lld%% time %d nps %lld\n", depth, nb_positions, total.nodes, total.quiescence_nodes, total.transposition_hits, total.beta_cutoffs, first_move_percentage, time_taken, nodes_per_second);

    // putting everything back
    search_iteration_callback = callback;
    forced_max_depth = 0;
    set_opening_book_usage(is_own_book_used, book_plies);
    reset_transposition_tables();
    set_start_position();

}


// Function to find the file of the opening book from the place of the program (<...>/UCI/UCI.x -> <...>/Alpha_Beta/Data/Opening_in_Games.txt)
// so the engine can be launched from any directory
static void find_opening_book_path(const char* program_path) {

    char real_path[PATH_MAX];
    if (program_path == NULL || realpath(program_path, real_path) == NULL) {
        return;
    }
    char* last_separator = strrchr(real_path, '/');
    if (last_separator == NULL) {
        return;
    }
    *last_separator = '\0';
    char book_path[PATH_MAX + 64];
    snprintf(book_path, sizeof(book_path), "%s/../Alpha_Beta/Data/Opening_in_Games.txt", real_path);
    if (access(book_path, R_OK) == 0) {
        set_opening_book_path(book_path);
    }

}


int main (int argc, char* argv[]) {

    // the standard output is kept for the answers of the protocol, the debug texts of the engine (its printf) are thrown away
    uci_output = fdopen(dup(STDOUT_FILENO), "w");
    if (uci_output == NULL) {
        return EXIT_FAILURE;
    }
    set_debug_output(false);

    // initialization of the engine
    create_game_settings();
    init_bitboards();
    create_log();
    create_bitboards_log();
    create_captured_pieces_and_score();
    initialize_pre_computed_data();
    initialize_random_transposition_tables();
    initialize_transposition_tables(hash_size_MB);
    initialize_attacks();
    find_opening_book_path(argv[0]);
    load_opening_book();
    set_opening_book_usage(is_own_book_used, book_plies);
    set_start_position();
    search_iteration_callback = print_search_info;

    // "./UCI.x bench [depth]" runs the bench and closes, to be used easily from a script
    if (argc >= 2 && strcmp(argv[1], "bench") == 0) {
        char command[64];
        snprintf(command, sizeof(command), "bench %s", (argc >= 3) ? argv[2] : "6");
        treat_bench_command(command);
        return EXIT_SUCCESS;
    }

    // reading the commands, one for each line
    static char line[UCI_LINE_SIZE];
    while (fgets(line, sizeof(line), stdin) != NULL) {

        // removing the end of the line and the spaces around the command
        line[strcspn(line, "\r\n")] = '\0';
        char* command = line;
        while (*command == ' ' || *command == '\t') {
            command++;
        }

        if (strcmp(command, "uci") == 0) {
            uci_print("id name %s %s\n", ENGINE_NAME, ENGINE_VERSION);
            uci_print("id author %s\n", ENGINE_AUTHOR);
            uci_print("option name Hash type spin default %d min 1 max 1024\n", DEFAULT_HASH_SIZE_MB);
            uci_print("option name OwnBook type check default true\n");
            uci_print("option name BookPlies type spin default %d min 0 max %d\n", DEFAULT_BOOK_PLIES, NUMBER_OF_OPENING_MOVES);
            uci_print("option name BookFile type string default <empty>\n");
            uci_print("option name Seed type spin default 0 min 0 max 2147483647\n");
            uci_print("option name Move Overhead type spin default %d min 0 max 5000\n", DEFAULT_MOVE_OVERHEAD_MS);
            uci_print("uciok\n");
        }
        else if (strcmp(command, "isready") == 0) {
            uci_print("readyok\n");
        }
        else if (strncmp(command, "setoption", 9) == 0) {
            stop_search();
            treat_setoption_command(command);
        }
        else if (strcmp(command, "ucinewgame") == 0) {
            stop_search();
            reset_transposition_tables();
            set_start_position();
        }
        else if (strncmp(command, "position", 8) == 0) {
            stop_search();
            treat_position_command(command);
        }
        else if (strncmp(command, "go", 2) == 0 && (command[2] == ' ' || command[2] == '\0')) {
            stop_search();
            treat_go_command(command);
        }
        else if (strcmp(command, "stop") == 0) {
            stop_search();
        }
        else if (strcmp(command, "quit") == 0) {
            break;
        }
        else if (strcmp(command, "d") == 0) {
            stop_search();
            print_position();
        }
        else if (strncmp(command, "bench", 5) == 0) {
            stop_search();
            treat_bench_command(command);
        }
        else if (strncmp(command, "debug", 5) == 0) {
            stop_search();
            set_debug_output(is_word_in_command(command, "on"));
        }
        // the unknown commands are ignored, as the protocol asks

    }

    // if the input is closed while a search is running, we let it finish to give its move (as with the command "quit" after a "go")
    if (is_search_thread_running) {
        if (feof(stdin) && !is_search_without_limit) {
            pthread_join(search_thread, NULL);
            is_search_thread_running = false;
        }
        else {
            stop_search();
        }
    }

    destroy_game_settings();
    destroy_bitboards();
    destroy_log();
    destroy_bitboards_log();
    free_pre_computed_data();
    free_random_transposition_tables();
    free_transposition_tables();
    free_opening_book();
    fclose(uci_output);

    return EXIT_SUCCESS;
}
