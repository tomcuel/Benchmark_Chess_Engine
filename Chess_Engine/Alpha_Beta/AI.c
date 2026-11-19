#include "AI.h"


void run_best_AI_level(move* selected_move, int* selected_score, bool* is_move_taken_from_book, int* depth_reached){

    // looking in the book opening if the move is in the book
    move move_from_book = get_move_from_book();
    if (!is_move_null(move_from_book)){
        *selected_move = move_from_book;
        *is_move_taken_from_book = true;
    }
    else {
        // running the iterative
        run_iterative_deepening(selected_move, selected_score, depth_reached);
    }

    // no move has been found (the game is over : checkmate or stalemate), so there is nothing to play
    if (is_move_null(*selected_move)){
        return;
    }

    // getting if a capture will happen, to update the captured pieces and the score
    int taken_piece = NOTHING;
    if (get_flag(*selected_move) == En_Passant_Flag){
        taken_piece = get_type_of_piece(bitboards, (game_settings->color_of_player_that_is_playing == WHITE) ? get_to_square(*selected_move) - 8 : get_to_square(*selected_move) + 8);
    }
    else if (will_capture(game_settings->color_of_player_that_is_playing, bitboards, get_to_square(*selected_move))){
        taken_piece = get_type_of_piece(bitboards, get_to_square(*selected_move));
    }
    update_captured_piece_and_score(taken_piece);

    // making the move
    make_move_AI(*selected_move);

}


void make_AI_play(move* selected_move, int* selected_score, bool* is_move_taken_from_book, int* depth_reached){

    // resetting the selected move and the selected score, as well as the boolean to know if the move is taken from the opening book before running the best AI
    *selected_move = 0;
    *selected_score = 0;
    *is_move_taken_from_book = false;
    *depth_reached = 0;
    run_best_AI_level(selected_move, selected_score, is_move_taken_from_book, depth_reached);

    // adding the biboard to the log to be able to detect draws after
    add_bitboards_to_log(bitboards);

}
