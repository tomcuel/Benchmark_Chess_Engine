#include "Search.h"


// setting the variables for the iterative deepening
move best_move;
int best_score;
move best_move_this_iteration;
int best_score_this_iteration;
int think_time_limit; // for the depth 1 search so it's not stopped
int start_time;
bool is_search_canceled;
int forced_think_time_in_ms = 0; // when it's positive, the search lasts this time instead of the one chosen with the clock of the game
int forced_max_depth = 0; // when it's positive, the iterative deepening stops after this depth
volatile bool is_search_stop_requested = false; // set to true from outside of the search to stop it as soon as possible
void (*search_iteration_callback)(int depth, int score, move best_move_found) = NULL; // called each time a depth is completed
Search_Statistics search_statistics = {0, 0, 0, 0, 0}; // the counters of the last search
bool is_draw_detection_enabled = true; // to see the repetitions of position and the 50 moves rule as draws in the search

// the history of the positions, to be able to detect the draws by repetition and by the 50 moves rule in the search
// both arrays are indexed by the size of the log : the index n is the position reached after n elements of the log
static bit_board position_keys[max_size_log_array + 1]; // the zobrist key of each position (0 when it's not known)
static int position_halfmove_clocks[max_size_log_array + 1]; // the number of half-moves without any capture or pawn move before each position
static int position_history_start; // the first index from which the keys are known
#define MAX_HISTORY_LOOKED_BACK 100 // after 100 half-moves without capture or pawn move, the game is a draw anyway


// Function to know if the search must stop (the time given is over, or the stop has been asked from outside)
// reading the clock at each position costs time, so it's only read once every 1024 positions (about every 2 milliseconds)
static bool is_search_time_over(){

    if (is_search_stop_requested){
        return true;
    }
    if ((search_statistics.nodes & 1023) != 0){
        return false;
    }
    return get_time_ms() - start_time > think_time_limit;

}


// Function to know if the last move of the log can't be taken back by the following moves (capture or pawn move)
// no position from before such a move can be reached again
static bool is_last_move_irreversible(){

    Log_Element* last_move = chess_log->moves[chess_log->actual_size-1];
    if (last_move->captured_piece_type != NOTHING || is_promotion_flag(get_flag(last_move->move_value))){
        return true;
    }
    int moved_piece_type = get_type_of_piece(bitboards, get_to_square(last_move->move_value));
    return (moved_piece_type == PAWN_WHITE || moved_piece_type == PAWN_BLACK);

}


// Function to fill the history of the positions of the game before a search
// The keys of the previous positions aren't kept during the game, so the last moves are undone then played again
// (only back to the last capture or pawn move, since no position from before it can come back)
static void prepare_position_history(){

    int size_at_root = chess_log->actual_size;
    position_history_start = size_at_root;
    position_halfmove_clocks[size_at_root] = 0;
    if (!is_draw_detection_enabled || size_at_root == 0){
        return;
    }

    // keeping a copy of the state of the game, to check that nothing has changed at the end
    bit_board bitboards_at_root[NUMBER_OF_BIBOARD];
    memcpy(bitboards_at_root, bitboards, sizeof(bitboards_at_root));
    Log_Element moves_at_root[MAX_HISTORY_LOOKED_BACK];
    int color_at_root = game_settings->color_of_player_that_is_playing;
    int player_at_root = game_settings->player_that_is_playing;

    // undoing the last moves, back to the last one that can't be taken back
    int nb_moves_undone = 0;
    bool is_stopped_by_irreversible_move = false;
    while (chess_log->actual_size > log_setup_size && nb_moves_undone < MAX_HISTORY_LOOKED_BACK){
        if (is_last_move_irreversible()){
            is_stopped_by_irreversible_move = true;
            break;
        }
        moves_at_root[nb_moves_undone] = *chess_log->moves[chess_log->actual_size-1];
        nb_moves_undone++;
        undo_last_move_AI();
    }

    // the oldest position looked at
    position_history_start = chess_log->actual_size;
    position_keys[position_history_start] = (position_history_start > 0) ? get_zobrist_key() : 0;
    // if we went back to the position from which the log starts, its number of half-moves is the one given with it
    position_halfmove_clocks[position_history_start] = (is_stopped_by_irreversible_move || chess_log->actual_size > log_setup_size) ? 0 : log_setup_halfmove_clock;

    // playing the moves again, and keeping the key of each position
    for (int i = nb_moves_undone-1; i >= 0; i--){
        make_move_AI(moves_at_root[i].move_value);
        position_keys[chess_log->actual_size] = get_zobrist_key();
        position_halfmove_clocks[chess_log->actual_size] = position_halfmove_clocks[chess_log->actual_size-1] + 1;
    }

    // the game must be exactly in the same state as before, otherwise we put it back and the draws aren't looked at for this search
    if (chess_log->actual_size != size_at_root || game_settings->color_of_player_that_is_playing != color_at_root || memcmp(bitboards_at_root, bitboards, sizeof(bitboards_at_root)) != 0){
        fprintf(stderr, "Error: the history of the positions couldn't be rebuilt, the draws by repetition are ignored for this search\n");
        memcpy(bitboards, bitboards_at_root, sizeof(bitboards_at_root));
        for (int i = 0; i < nb_moves_undone; i++){
            *chess_log->moves[size_at_root-1-i] = moves_at_root[i];
        }
        chess_log->actual_size = size_at_root;
        game_settings->color_of_player_that_is_playing = color_at_root;
        game_settings->player_that_is_playing = player_at_root;
        position_history_start = size_at_root;
        position_halfmove_clocks[size_at_root] = 0;
    }

}


// Function to know if the current position has already been reached, in the game or in the line that is being searched
// one repetition is enough to consider the position as a draw : if a player can repeat once, he can repeat again
static bool is_position_repeated(bit_board zobrist_key){

    int size = chess_log->actual_size;
    // going back in the positions, the index p is the position before the move p of the log
    for (int p = size-1; p >= position_history_start; p--){
        // a capture can't be taken back, the positions from before it can't be the same as the current one
        if (chess_log->moves[p]->captured_piece_type != NOTHING){
            return false;
        }
        // only the positions where the same player is playing can be the same
        if (((size - p) & 1) == 0 && position_keys[p] == zobrist_key){
            return true;
        }
    }
    return false;

}


int choose_think_time(int color_playing){

    int ms = 1000; // 1 second

    int time_remaining = (color_playing == WHITE) ? game_settings->white_time_remaining : game_settings->black_time_remaining;
    int time_at_start = game_settings->time_at_start;
    // more than 30sec for 10min, 1min30 for 30min, 3min for 60min;
    if (time_remaining < (time_at_start/20)){ 
        return (time_remaining/10)*ms;
    }
    
    return 5000; //(time_at_start/30)*ms;
}


void run_iterative_deepening(move* selected_move, int* selected_score, int* depth_reached){

    printf("iterative deepening with transposition tables and search extensions\n");

    // if there is no move to play (checkmate or stalemate), there is nothing to search and the selected move stays null
    int nb_moves_at_root = 0;
    move* moves_at_root = generate_all_possible_moves(&nb_moves_at_root);
    free(moves_at_root);
    if (nb_moves_at_root == 0){
        return;
    }

    // setting the variables for the iterative deepening
    is_search_canceled = false;
    best_move = 0;
    best_score = 0;
    best_move_this_iteration = 0;
    best_score_this_iteration = INT_MIN;

    // resetting the counters of the search
    search_statistics.nodes = 0;
    search_statistics.quiescence_nodes = 0;
    search_statistics.transposition_hits = 0;
    search_statistics.beta_cutoffs = 0;
    search_statistics.first_move_beta_cutoffs = 0;

    // getting the keys of the previous positions of the game, to detect the repetitions
    prepare_position_history();

    // the entries of the transposition tables written by the previous searches can now be replaced
    start_new_search_in_TT();

    start_time = get_time_ms(); // the time when the search started, initialized in the run_iterative_deepening function
    think_time_limit = (forced_think_time_in_ms > 0) ? forced_think_time_in_ms : choose_think_time(game_settings->color_of_player_that_is_playing); 
    
    // running the iterative deepening (depth one is already done in the make play function)
    int max_depth = (forced_max_depth > 0 && forced_max_depth < MAX_DEPTH_ITERATIVE_DEEPENING) ? forced_max_depth : MAX_DEPTH_ITERATIVE_DEEPENING - 1;
    for (int search_depth = 1; search_depth <= max_depth; search_depth++){

        // making the search for the best move for this AI level
        int score = Search(search_depth, 0, NEGATIVE_INFINITY, POSITIVE_INFINITY, 0);
        printf("score : %d for move from %d to %d for depth %d\n", score, get_from_square(best_move_this_iteration), get_to_square(best_move_this_iteration), search_depth);
        *depth_reached = search_depth;

        // if the search has been canceled, we need to stop the iterative deepening search
        if (is_search_canceled){ 
            break; // stop the search when the move score is 0 or best_move_this_iteration is 0 but without having to verify it here
        }
        // otherwise, we need to update the best move found since it's sure that we have found a move
        else {
            
            best_move = best_move_this_iteration;
            best_score = best_score_this_iteration;
            best_move_this_iteration = 0;
            best_score_this_iteration = INT_MIN;

            // telling that a depth has been completed, to whom is interested (the UCI interface for example)
            if (search_iteration_callback != NULL && !is_move_null(best_move)){
                search_iteration_callback(search_depth, best_score, best_move);
            }

            if (abs(best_score) > IMMEDIATE_MATE_SCORE - 1000){
                break; // stop the search when the move score make a mate since there is not need to look further
            }
        }

    }

    // udpating the selected move if it's not null
    if (!is_move_null(best_move)){
        *selected_move = best_move;
        *selected_score = best_score;
        printf("best score : %d for move from %d to %d\n", best_score, get_from_square(best_move), get_to_square(best_move));
        printf("\n");
    }
    // back_up in case we didn't find any move, generally no used, time needs to be less than 100ms to reach this point, and it's only a one or two positions after the opening moves
    else {
        int backup_search_time = get_time_ms();
        is_search_canceled = false;
        is_search_stop_requested = false; // this search is needed to have a move to play
        think_time_limit = INT_MAX; // for the depth 1 search so it's not stopped
        Search(1, 0, NEGATIVE_INFINITY, POSITIVE_INFINITY, 0);
        int time_passed_during_backup_search = get_time_ms() - backup_search_time;
        printf("time passed during backup search : %d\n", time_passed_during_backup_search);
        *selected_move = best_move_this_iteration;
        *selected_score = best_score_this_iteration;
        printf("best score : %d for move from %d to %d, depth 1 recup\n", best_score_this_iteration, get_from_square(best_move_this_iteration), get_to_square(best_move_this_iteration));

    }

    // resetting the variables for the iterative deepening
    is_search_canceled = false;
    
}


int Search(int ply_remaining, int ply_from_root, int alpha, int beta, int nb_extensions){

    // if the search is canceled, we need to stop the search
    if (is_search_canceled){
        return 0;
    }

    // before making the search, we need to check if the time left to think is not over
    if (is_search_time_over()){
        is_search_canceled = true;
        return 0;
    }

    search_statistics.nodes++;

    // making some cut off to continue the alpha beta pruning 
    if (ply_from_root > 0){
        alpha = max(alpha, -IMMEDIATE_MATE_SCORE + ply_from_root);
        beta = min(beta, IMMEDIATE_MATE_SCORE - ply_from_root);
        // cut off if necessary
        if (alpha >= beta){
            return alpha;
        }
    }

    // looking if we have already evaluated this position in the transposition tables
    bit_board hashed_zobrist_key = 0;
    int index_in_TT = 0;
    if (chess_log->actual_size > 0){
        hashed_zobrist_key = get_zobrist_key();

        // a position that has already been reached, or 50 moves without any capture or pawn move, is a draw
        // it's looked at before the transposition tables, because the value kept there doesn't know the positions already played
        if (is_draw_detection_enabled && chess_log->actual_size <= max_size_log_array){
            position_keys[chess_log->actual_size] = hashed_zobrist_key;
            if (ply_from_root > 0 && (position_halfmove_clocks[chess_log->actual_size] >= 100 || is_position_repeated(hashed_zobrist_key))){
                return 0;
            }
        }

        index_in_TT = get_index_zobrist_in_TT(hashed_zobrist_key);
        int TT_value = lookup_for_evaluation_in_TT(index_in_TT, hashed_zobrist_key, ply_remaining, ply_from_root, alpha, beta);
        if (TT_value != FAILED_LOOKUP){
            search_statistics.transposition_hits++;
            if (ply_from_root == 0){
                best_move_this_iteration = transposition_tables->entries[index_in_TT].move;
                best_score_this_iteration = TT_value;
            }
            return TT_value;
        }
    }

    if (ply_remaining == 0){
        // making the search for the captures happening afterwards (the quiescence search)
        return Quiescience_Search(alpha, beta);
    }

    // generating all the possible moves for the AI at this point and ordering them to make the search faster, because we will search the best moves first
    int nb_moves = 0; 
    move* valid_moves = generate_all_possible_moves(&nb_moves);
    // if there are no moves, we need to check if the king is in check, if it is, we return the worst score possible since it's a mate, else we return 0 since it's a draw
    if (nb_moves == 0){
        if (is_check(game_settings->color_of_player_that_is_playing, bitboards, chess_log) == CHECK && get_king_index(bitboards, game_settings->color_of_player_that_is_playing) != -1){
            free(valid_moves);
            // the mate is worse when it's closer to the root of the search : the AI goes for the fastest mate, and delays the one it suffers
            return -(IMMEDIATE_MATE_SCORE - ply_from_root);
        }
        free(valid_moves);
        return 0;
    }

    // adding some variables for the transposition tables
    int cut_flag = ALPHA_CUT;
    move best_move_in_this_position = 0;

    // ordering the moves to make the search faster
    // the best move kept in the transposition tables for this position (even from a search not deep enough to give its value) is tried first
    move hash_move = 0;
    if (chess_log->actual_size > 0 && hashed_zobrist_key != 0 && transposition_tables->entries[index_in_TT].zobrist_key == hashed_zobrist_key){
        hash_move = transposition_tables->entries[index_in_TT].move;
    }
    order_moves(false, valid_moves, nb_moves, hash_move);
    // searching for the best move by evaluating the position after each move
    for (int i = 0; i < nb_moves; i++){

        int valid_move_to = get_to_square(valid_moves[i]);
        int flag = get_flag(valid_moves[i]);

        // getting if a capture will happen, used later 
        bool is_capture_happening = will_capture(game_settings->color_of_player_that_is_playing, bitboards, valid_move_to);
        if (flag == En_Passant_Flag){
            is_capture_happening = true;
        }

        // a capture or a pawn move puts back to 0 the number of half-moves of the 50 moves rule
        int piece_type_moving = get_type_of_piece(bitboards, get_from_square(valid_moves[i]));
        bool is_move_irreversible = is_capture_happening || piece_type_moving == PAWN_WHITE || piece_type_moving == PAWN_BLACK;

        // making the move first
        make_move_AI(valid_moves[i]);
        if (chess_log->actual_size <= max_size_log_array){
            position_halfmove_clocks[chess_log->actual_size] = is_move_irreversible ? 0 : position_halfmove_clocks[chess_log->actual_size-1] + 1;
        }

        // Extend the depth of the search in certain interesting cases
        int extension = 0;
        if (nb_extensions < MAX_NUMBER_OF_EXTENSIONS){

            // getting some information to know if we need to extend the search
            int targeted_rank = get_rank(valid_move_to);
            int moved_piece_type = get_type_of_piece(bitboards, valid_move_to); // the move has already been made, the piece is on its arrival square

            // a check is interesting to look further
            if (is_check(game_settings->color_of_player_that_is_playing, bitboards, chess_log) == CHECK){
                extension = 1;
            }
            // it's also interesting to look further if a promotion is near
            else if ((moved_piece_type == PAWN_WHITE || moved_piece_type == PAWN_BLACK) && (targeted_rank == 1 || targeted_rank == 6)){
                extension = 1;
            }

        }

        bool need_full_search = true;
        int eval = 0;

        // Reduce the depth of the search for moves later in the move list as these are less likely to be good (assuming our move ordering isn't terrible)
        if (extension == 0 && ply_remaining >= 3 && i >= 3 && !is_capture_happening){
            // here is our reduced depth
            int reduced_depth = 1;
            // calculating the evaluation of the position at the reduced depth
            eval = -Search(ply_remaining-1-reduced_depth, ply_from_root+1, -alpha - 1, -alpha, nb_extensions);
            // If the evaluation is better than expected, we'd better to a full-depth search to get a more accurate evaluation
			need_full_search = (eval > alpha);
        }
    
        // Perform a full-depth search
        if (need_full_search){
            // calculating the evaluation of the position at the full depth (extended by the interesting cases)
            eval = -Search(ply_remaining-1+extension, ply_from_root+1, -beta, -alpha, nb_extensions+extension);
        }

        // undoing the move to make the search and not modify the game state
        undo_last_move_AI();

        // if the search is canceled, during the search at depth-1, we need to stop the search
        if (is_search_canceled){
            free(valid_moves);
            return 0;
        }
        
        // pruning the search if the score is better than the beta value
        if (eval >= beta){
            search_statistics.beta_cutoffs++;
            if (i == 0){
                search_statistics.first_move_beta_cutoffs++;
            }
            store_evaluation_in_TT(index_in_TT, hashed_zobrist_key, ply_remaining, ply_from_root, beta, BETA_CUT, valid_moves[i]);
            free(valid_moves);
            return beta;
        }
        // updating the alpha value if the score is better than the current alpha value
        if (eval > alpha){
            alpha = eval;

            // udpating the transposition tables variables
            cut_flag = EXACT;
            best_move_in_this_position = valid_moves[i];
            
            // if we're at the first depth (the root here), we need to update the selected move, it's the best move found yet
            if (ply_from_root == 0){
                best_move_this_iteration = valid_moves[i];
                best_score_this_iteration = eval;
            }
        }

    }

    // storing the evaluation in the transposition tables
    store_evaluation_in_TT(index_in_TT, hashed_zobrist_key, ply_remaining, ply_from_root, alpha, cut_flag, best_move_in_this_position);

    // returning the lower bound of the score if nothing better has been found
    free(valid_moves);
    return alpha;

}


int Quiescience_Search(int alpha, int beta){

    // before making the search, we need to check if the time left to think is not over
    if (is_search_time_over()){
        is_search_canceled = true;
        return 0;
    }

    // if the search is canceled, we need to stop the search
    if (is_search_canceled){
        return 0;
    }

    search_statistics.nodes++;
    search_statistics.quiescence_nodes++;

    // evaluating the position
    int evaluation = evaluate();
    // pruning the search if the evaluation is better than the beta value
    if (evaluation >= beta){
       return beta;
    }
    // updating the alpha value if the evaluation is better than the current alpha value
    if (evaluation > alpha){
        alpha = evaluation;
    }

    // since we're dealing with captures, we need to check if the king is still on the board, if not, we return the worst score possible since it's a mate
    if (get_king_index(bitboards, game_settings->color_of_player_that_is_playing) == -1){
        return -MAX_IA_SCORE;
    }

    // generating all the possible moves that captures for the AI at this point and ordering them to make the search faster, because we will search the best moves first
    int nb_moves = 0; 
    move* valid_moves = generate_all_possible_moves_that_capture(&nb_moves);
    // if there is no capture left, we return the evaluation
    if (nb_moves == 0){
        free(valid_moves);
        return evaluation;
    }

    // ordering the moves to make the search faster (we only order the captures)
    order_moves(true, valid_moves, nb_moves, 0);
    // searching for the best move by evaluating the position after each move
    for (int i = 0; i < nb_moves; i++){

        // making the move, getting the score of the position after the move (updating the evaluation), and undoing the move
        make_move_AI(valid_moves[i]);
        evaluation = -Quiescience_Search(-beta, -alpha);
        undo_last_move_AI();
        
        // if the search is canceled, during the search at depth-1, we need to stop the search
        if (is_search_canceled){
            free(valid_moves);
            return 0;
        }
        
        // pruning the search if the evaluation is better than the beta value
        if (evaluation >= beta){
            free(valid_moves);
            return beta;
        }
        // updating the alpha value if the evaluation is better than the current alpha value
        if (evaluation > alpha){
            alpha = evaluation;
        }

    }

    // returning the lower bound of the score if nothing better has been found
    free(valid_moves);
    return alpha;

}
