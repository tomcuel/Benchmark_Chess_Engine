#include "../Alpha_Beta/Pre_Computed_Data.h"
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
// Perft : program to check the move generation of the chess engine
// It counts all the positions that can be reached in a given number of half-moves from a position,
// and compares the result to the known values of some reference positions (https://www.chessprogramming.org/Perft_Results)
// If the numbers are the same, the generation of the moves (and the way to make and undo them) follows the rules of chess
/**
 * usage :
 * ./Perft.x                      : runs all the reference positions, and says if the move generation is correct
 * ./Perft.x <depth> "<fen>"      : counts the positions from the given FEN, with the detail for each first move (to find where a difference comes from)
**/
///////////////////////////////////////////////////////////////////////////////


// A reference position, with the number of positions expected at each depth (0 at the end of the list)
typedef struct Perft_Position {
    const char* name;
    const char* fen;
    long long expected[8];
} Perft_Position;


static const Perft_Position reference_positions[] = {
    {"start position", START_POSITION_FEN, {20, 400, 8902, 197281, 4865609, 0}},
    {"castles, pins, en passant (Kiwipete)", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", {48, 2039, 97862, 4085603, 0}},
    {"endgame with en passant and pins", "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", {14, 191, 2812, 43238, 674624, 11030083, 0}},
    {"promotions and checks", "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", {6, 264, 9467, 422333, 0}},
    {"promotions and checks (mirrored)", "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1", {6, 264, 9467, 422333, 0}},
    {"promotion with capture and castle", "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", {44, 1486, 62379, 2103487, 0}},
    {"middle game", "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", {46, 2079, 89890, 3894594, 0}},
};


// Function to count the positions that can be reached in depth half-moves from the current position
static long long perft(int depth) {

    int nb_moves = 0;
    move* valid_moves = generate_all_possible_moves(&nb_moves);
    // at the last half-move, the number of positions is the number of legal moves
    if (depth <= 1) {
        free(valid_moves);
        return nb_moves;
    }
    long long nb_positions = 0;
    for (int i = 0; i < nb_moves; i++) {
        make_move_AI(valid_moves[i]);
        nb_positions += perft(depth - 1);
        undo_last_move_AI();
    }
    free(valid_moves);
    return nb_positions;

}


// Function to write a move in the UCI notation (ex : e2e4, e7e8q)
static void get_move_name(move move_value, char* name) {

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


int main (int argc, char* argv[]) {

    // initialization of the engine (only what the move generation needs)
    create_game_settings();
    init_bitboards();
    create_log();
    create_bitboards_log();
    create_captured_pieces_and_score();
    initialize_pre_computed_data();
    initialize_random_transposition_tables();
    initialize_attacks();
    // all the legal moves are counted, the promotions to a rook, a bishop and a knight included
    are_under_promotions_generated = true;

    int exit_code = EXIT_SUCCESS;

    // counting from a given position, with the detail for each first move
    if (argc >= 3) {
        int depth = atoi(argv[1]);
        if (depth < 1 || !set_position_from_fen(argv[2])) {
            fprintf(stderr, "usage : %s <depth> \"<fen>\" (the depth must be at least 1 and the FEN valid)\n", argv[0]);
            return EXIT_FAILURE;
        }
        int nb_moves = 0;
        move* valid_moves = generate_all_possible_moves(&nb_moves);
        long long total = 0;
        for (int i = 0; i < nb_moves; i++) {
            char name[6];
            get_move_name(valid_moves[i], name);
            long long nb_positions = 1;
            if (depth > 1) {
                make_move_AI(valid_moves[i]);
                nb_positions = perft(depth - 1);
                undo_last_move_AI();
            }
            printf("%s: %lld\n", name, nb_positions);
            total += nb_positions;
        }
        free(valid_moves);
        printf("\nNodes searched: %lld\n", total);
    }
    // running all the reference positions
    else {
        int nb_positions_tested = sizeof(reference_positions) / sizeof(reference_positions[0]);
        int nb_errors = 0;
        long long total_nodes = 0;
        int time_at_start = get_time_ms();
        for (int i = 0; i < nb_positions_tested; i++) {
            printf("%s\n%s\n", reference_positions[i].name, reference_positions[i].fen);
            if (!set_position_from_fen(reference_positions[i].fen)) {
                printf("  the FEN couldn't be read\n");
                nb_errors++;
                continue;
            }
            for (int depth = 1; reference_positions[i].expected[depth-1] != 0; depth++) {
                long long nb_positions = perft(depth);
                long long expected = reference_positions[i].expected[depth-1];
                total_nodes += nb_positions;
                if (nb_positions == expected) {
                    printf("  depth %d : %lld ok\n", depth, nb_positions);
                }
                else {
                    printf("  depth %d : %lld ERROR (expected %lld)\n", depth, nb_positions, expected);
                    nb_errors++;
                }
            }
        }
        int time_taken = get_time_ms() - time_at_start;
        printf("\n%lld positions counted in %d ms\n", total_nodes, time_taken);
        if (nb_errors == 0) {
            printf("the move generation is correct on all the reference positions\n");
        }
        else {
            printf("%d differences found with the reference values\n", nb_errors);
            exit_code = EXIT_FAILURE;
        }
    }

    destroy_game_settings();
    destroy_bitboards();
    destroy_log();
    destroy_bitboards_log();
    free_pre_computed_data();
    free_random_transposition_tables();

    return exit_code;
}
