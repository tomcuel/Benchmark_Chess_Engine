#include "../Alpha_Beta/AI.h"


///////////////////////////////////////////////////////////////////////////////
// Small program to check that the chess engine works : it plays against itself in the terminal
// It loads the opening book, then makes the AI play the first moves of a game against itself
/**
 * usage (from the root of the project) : ./Chess_Engine/UCI/Engine_Check.x [think time in ms] [number of half-moves] [seed]
**/
///////////////////////////////////////////////////////////////////////////////
int main (int argc, char* argv[]) {

    int think_time = (argc > 1) ? atoi(argv[1]) : 500;
    int nb_plies = (argc > 2) ? atoi(argv[2]) : 14;
    if (argc > 3) {
        set_fixed_seed((unsigned int)atoi(argv[3]));
    }

    // initialization of the engine
    create_game_settings();
    init_bitboards();
    create_log();
    create_bitboards_log();
    create_captured_pieces_and_score();
    initialize_pre_computed_data();
    initialize_random_transposition_tables();
    initialize_transposition_tables(64);
    initialize_attacks();
    load_opening_book();
    printf("opening book : %d lines loaded\n", get_number_of_opening_book_lines());

    // the book is used for the 5 first moves of each side, then the search takes over
    set_opening_book_usage(true, 10);
    forced_think_time_in_ms = think_time;

    for (int ply = 0; ply < nb_plies; ply++) {
        move selected_move = 0;
        int selected_score = 0;
        bool is_move_taken_from_book = false;
        int depth_reached = 0;
        int time_before = get_time_ms();
        make_AI_play(&selected_move, &selected_score, &is_move_taken_from_book, &depth_reached);
        int time_taken = get_time_ms() - time_before;
        if (is_move_null(selected_move)) {
            printf("ply %d : no move found\n", ply+1);
            break;
        }
        int from = get_from_square(selected_move);
        int to = get_to_square(selected_move);
        printf("ply %d : %c%d%c%d %s (score %d, depth %d, %d ms)\n", ply+1, 'a'+get_file(from), 1+get_rank(from), 'a'+get_file(to), 1+get_rank(to), is_move_taken_from_book ? "book" : "search", selected_score, depth_reached, time_taken);
    }

    destroy_game_settings();
    destroy_bitboards();
    free_pre_computed_data();
    free_random_transposition_tables();
    free_transposition_tables();
    free_opening_book();

    return EXIT_SUCCESS;
}
