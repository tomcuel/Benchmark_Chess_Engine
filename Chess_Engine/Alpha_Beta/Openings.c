#include "Openings.h"


// the opening book is kept in memory : one line per game, with the NUMBER_OF_OPENING_MOVES first moves of the game
// nothing is written on the disk, so several engines can run at the same time without disturbing each other
static move (*opening_book_lines)[NUMBER_OF_OPENING_MOVES] = NULL;
static int nb_opening_book_lines = 0;
// the file with the openings in formal chess language (the path is relative to where the program is launched)
static char opening_book_path[PATH_MAX] = "Chess_Engine/Alpha_Beta/Data/Opening_in_Games.txt";
// the way the opening book is used
static bool is_opening_book_enabled = true;
static int opening_book_max_plies = NUMBER_OF_OPENING_MOVES;


void set_opening_book_path(const char* path){

    snprintf(opening_book_path, sizeof(opening_book_path), "%s", path);

}


void set_opening_book_usage(bool is_enabled, int max_plies){

    is_opening_book_enabled = is_enabled;
    opening_book_max_plies = int_clamp(max_plies, 0, NUMBER_OF_OPENING_MOVES);

}


int get_number_of_opening_book_lines(){

    return nb_opening_book_lines;

}


void free_opening_book(){

    free(opening_book_lines);
    opening_book_lines = NULL;
    nb_opening_book_lines = 0;

}


// Function to know if a piece can legally go from a square to another one in the current position (used to check the moves read in the book)
static bool is_from_to_playable(int from_square, int to_square){

    if (from_square < 0 || from_square > 63 || to_square < 0 || to_square > 63){
        return false;
    }
    int nb_moves = 0;
    move* valid_moves = generate_all_possible_moves(&nb_moves);
    bool is_playable = false;
    for (int i = 0; i < nb_moves; i++){
        if (get_from_square(valid_moves[i]) == from_square && get_to_square(valid_moves[i]) == to_square){
            is_playable = true;
            break;
        }
    }
    free(valid_moves);
    return is_playable;

}


void load_opening_book(){

    // if the book was already loaded, we start again from scratch
    free_opening_book();

    // Opening the file with all the first moves of the games in formal chess language
    // one thing here that isn't considered is to have two knights for example that goes to the same square, we need to add the rank or file from where it comes, but I don't do that and simply suppress the lines where it happens
    FILE* formal_file = fopen(opening_book_path, "r");
    // If the file doesn't exist, we need to stop the function (the AI will simply play without the opening book)
    if (formal_file == NULL) {
        fprintf(stderr, "Error: Unable to open the opening book '%s'\n", opening_book_path);
        return;
    }

    // reading the file with the openings in formal chess language
    int capacity = 0;
    int nb_skipped_lines = 0;
    char opening_move[16];
    move line_moves[NUMBER_OF_OPENING_MOVES];
    bool is_file_empty = false;
    while (!is_file_empty) {
        int nb_moves_made = 0;
        bool is_line_valid = true;
        for (int i = 0; i < NUMBER_OF_OPENING_MOVES; i++){
            // the end of the file and we break
            if (fscanf(formal_file, "%15s", opening_move) != 1){
                is_file_empty = true;
                break;
            }
            // a move of the line wasn't understood, we only read the rest of the line to go to the next one
            if (!is_line_valid){
                continue;
            }
            move move_value = 0;
            int from_square = 0;
            int to_square = 0;
            int flag = No_Flag;
            // if the move is a castle
            if (strcmp(opening_move, "O-O") == 0){
                from_square = (game_settings->color_of_player_that_is_playing == WHITE) ? 4 : 60;
                to_square = (game_settings->color_of_player_that_is_playing == WHITE) ? 6 : 62;
                flag = Castle_Flag;
            }
            else if (strcmp(opening_move, "O-O-O") == 0){
                from_square = (game_settings->color_of_player_that_is_playing == WHITE) ? 4 : 60;
                to_square = (game_settings->color_of_player_that_is_playing == WHITE) ? 2 : 58;
                flag = Castle_Flag;
            }
            else {
                // there is no promotion in the opening book since 3/4 moves can't lead to a promotion (5 would have)
                
                // it's the index of the case square
                int index = 0;
                char from_file = 'a';
                bool is_there_a_file_for_the_piece = false;
                if (opening_move[0] == 'N' || opening_move[0] == 'B' || opening_move[0] == 'R' || opening_move[0] == 'Q' || opening_move[0] == 'K'){
                    index = 1;
                    // if there are a file that is specified for the piece, just need to take that into account, because if two pieces can move to the same square, we need to know from where it comes (ex : Nfc3)
                    if (opening_move[1] >= 'a' && opening_move[1] <= 'h' && opening_move[2] >= 'a' && opening_move[2] <= 'h'){
                        is_there_a_file_for_the_piece = true;
                        from_file = opening_move[1];
                        index = 2;
                    }
                }
                
                // if a capture is happening (ex : exd4 or Nfxd4)
                if (opening_move[1] == 'x' || opening_move[2] == 'x'){
                    index = 2; 
                    if (is_there_a_file_for_the_piece){
                        index = 3;
                    }
                }
                // getting the square to where the piece is moving
                int to_rank = (int)(opening_move[index+1]-'1');
                int to_file = (int)(opening_move[index]-'a');
                // the square isn't on the board, the notation of the move isn't handled, the line will be skipped
                if (to_rank < 0 || to_rank > 7 || to_file < 0 || to_file > 7){
                    is_line_valid = false;
                    nb_skipped_lines++;
                    continue;
                }
                to_square = get_index(to_rank, to_file);
                
                // now we need to get from where the piece moved
                // if the move is a knight move
                if (opening_move[0] == 'N'){
                    bit_board knight_mask = precomputed_knight_attacks[to_square];
                    while (knight_mask){
                        int temp_from_square = __builtin_ctzll(knight_mask);
                        if (bitboards[game_settings->color_of_player_that_is_playing == WHITE ? KNIGHT_WHITE : KNIGHT_BLACK] & (1ULL << temp_from_square)){
                            // if we're in the Nfc3 case, we need to check if the file is the good one, because two knights can move to the same square
                            if (is_there_a_file_for_the_piece){
                                if (get_file(temp_from_square) == (int)(from_file-'a')){
                                    from_square = temp_from_square;
                                    break;
                                }
                            }
                            // no Nfc3 case, so we do the basic stuff
                            else {
                                from_square = temp_from_square;
                                break;
                            }
                        }
                        knight_mask &= knight_mask - 1;
                    } 
                }
                // if the move is a bishop move
                else if (opening_move[0] == 'B'){
                    bit_board bishop_mask = calculate_bishop_attacks_with_blockers(to_square, bitboards[WHITE_PIECES] | bitboards[BLACK_PIECES]);
                    while (bishop_mask){
                        int temp_from_square = __builtin_ctzll(bishop_mask);
                        if (bitboards[game_settings->color_of_player_that_is_playing == WHITE ? BISHOP_WHITE : BISHOP_BLACK] & (1ULL << temp_from_square)){
                            // if we're in the Bfc3 case, we need to check if the file is the good one, that we don't select the wrong bishop (can happen after a pawn promotion, because bishop don't cover the same squares)
                            if (is_there_a_file_for_the_piece){
                                if (get_file(temp_from_square) == (int)(from_file-'a')){
                                    from_square = temp_from_square;
                                    break;
                                }
                            }
                            // no Bfc3 case, so we do the basic stuff
                            else {
                                from_square = temp_from_square;
                                break;
                            }
                        }
                        bishop_mask &= bishop_mask - 1;
                    }
                }
                // if the move is a rook move
                else if (opening_move[0] == 'R'){
                    bit_board rook_mask = calculate_rook_attacks_with_blockers(to_square, bitboards[WHITE_PIECES] | bitboards[BLACK_PIECES]);
                    while (rook_mask){
                        int temp_from_square = __builtin_ctzll(rook_mask);
                        if (bitboards[game_settings->color_of_player_that_is_playing == WHITE ? ROOK_WHITE : ROOK_BLACK] & (1ULL << temp_from_square)){
                            // if we're in the Rfc3 case, we need to check if the file is the good one, that we don't select the other rook possible
                            if (is_there_a_file_for_the_piece){
                                if (get_file(temp_from_square) == (int)(from_file-'a')){
                                    from_square = temp_from_square;
                                    break;
                                }
                            }
                            // no Rfc3 case, so we do the basic stuff
                            else {
                                from_square = temp_from_square;
                                break;
                            }
                        }
                        rook_mask &= rook_mask - 1;
                    }
                }
                // if the move is a queen move
                else if (opening_move[0] == 'Q'){
                    bit_board queen_mask = calculate_bishop_attacks_with_blockers(to_square, bitboards[WHITE_PIECES] | bitboards[BLACK_PIECES]) | calculate_rook_attacks_with_blockers(to_square, bitboards[WHITE_PIECES] | bitboards[BLACK_PIECES]);
                    while (queen_mask){
                        int temp_from_square = __builtin_ctzll(queen_mask);
                        if (bitboards[game_settings->color_of_player_that_is_playing == WHITE ? QUEEN_WHITE : QUEEN_BLACK] & (1ULL << temp_from_square)){
                            // if we're in the Qfc3 case, we need to check if the file is the good one (can happen after a pawn promotion), so we don't select the wrong queen
                            if (is_there_a_file_for_the_piece){
                                if (get_file(temp_from_square) == (int)(from_file-'a')){
                                    from_square = temp_from_square;
                                    break;
                                }
                            }
                            // no Qfc3 case, so we do the basic stuff
                            else {
                                from_square = temp_from_square;
                                break;
                            }
                        }
                        queen_mask &= queen_mask - 1;
                    }
                }
                // if the move is a king move
                else if (opening_move[0] == 'K'){
                    bit_board king_mask = precomputed_king_attacks[to_square];
                    while (king_mask){
                        int temp_from_square = __builtin_ctzll(king_mask);
                        if (bitboards[game_settings->color_of_player_that_is_playing == WHITE ? KING_WHITE : KING_BLACK] & (1ULL << temp_from_square)){
                            // only one king so no file problem
                            from_square = temp_from_square;
                            break;
                        }
                        king_mask &= king_mask - 1;
                    }
                    
                }
                // if the move is a pawn move
                else {
                    if (game_settings->color_of_player_that_is_playing == WHITE){
                        
                        // if there is a standard capture 
                        if (opening_move[1] == 'x'){
                            // if there is a capture on the left
                            if ((bitboards[PAWN_WHITE] & (1ULL << (to_square - 9))) && (bitboards[BLACK_PIECES] & (1ULL << to_square)) && get_file(to_square) != 0 && get_file(to_square-9) == (int)(opening_move[0]-'a')){
                                from_square = to_square - 9;
                            }
                            // if there is a capture on the right
                            else if ((bitboards[PAWN_WHITE] & (1ULL << (to_square - 7))) && (bitboards[BLACK_PIECES] & (1ULL << to_square)) && get_file(to_square) != 7 && get_file(to_square-7) == (int)(opening_move[0]-'a')){
                                from_square = to_square - 7;
                            }
                            // if there is a left en_passant possible 
                            else if (is_en_passant_possible(WHITE, bitboards, to_square-9, to_square, chess_log)){
                                from_square = to_square - 9;
                                flag = En_Passant_Flag;
                            }
                            // if there is a right en_passant possible 
                            else if (is_en_passant_possible(WHITE, bitboards, to_square-7, to_square, chess_log)){
                                from_square = to_square - 7;
                                flag = En_Passant_Flag;
                            }
                        }
                        // if there is a simple move
                        else if (bitboards[PAWN_WHITE] & (1ULL << (to_square - 8))){
                            from_square = to_square - 8;
                        }
                        // if there is a double move
                        else if ((bitboards[PAWN_WHITE] & (1ULL << (to_square - 16))) && get_rank(to_square) == 3){
                            from_square = to_square - 16;
                            flag = Pawn_Two_Up_Flag;
                        }

                    }
                    else if (game_settings->color_of_player_that_is_playing == BLACK){
                        
                        // if there is a standard capture
                        if (opening_move[1] == 'x'){
                            // if there is a capture on the left
                            if ((bitboards[PAWN_BLACK] & (1ULL << (to_square + 7))) && (bitboards[WHITE_PIECES] & (1ULL << to_square)) && get_file(to_square) != 0 && get_file(to_square+7) == (int)(opening_move[0]-'a')){
                                from_square = to_square + 7;
                            }
                            // if there is a capture on the right
                            else if ((bitboards[PAWN_BLACK] & (1ULL << (to_square + 9))) && (bitboards[WHITE_PIECES] & (1ULL << to_square)) && get_file(to_square) != 7 && get_file(to_square+9) == (int)(opening_move[0]-'a')){
                                from_square = to_square + 9;
                            }
                              // if there is a left en_passant possible 
                            else if (is_en_passant_possible(BLACK, bitboards, to_square+7, to_square, chess_log)){
                                from_square = to_square + 7;
                                flag = En_Passant_Flag;
                            }
                            // if there is a right en_passant possible 
                            else if (is_en_passant_possible(BLACK, bitboards, to_square+9, to_square, chess_log)){
                                from_square = to_square + 9;
                                flag = En_Passant_Flag;
                            }
                        }
                        // if there is a simple move
                        else if (bitboards[PAWN_BLACK] & (1ULL << (to_square + 8))){
                            from_square = to_square + 8;
                        }
                        // if there is a double move
                        else if ((bitboards[PAWN_BLACK] & (1ULL << (to_square + 16))) && get_rank(to_square) == 4){
                            from_square = to_square + 16;
                            flag = Pawn_Two_Up_Flag;
                        }

                    }
                }

            }

            // the notation of some moves isn't handled (ex : N1c3, when the rank from where the piece comes is given)
            // so we check that the move found can really be played, otherwise the whole line is skipped
            if (!is_from_to_playable(from_square, to_square)){
                is_line_valid = false;
                nb_skipped_lines++;
                continue;
            }

            move_value = get_move_value(from_square, to_square, flag);
            make_move_AI(move_value);
            line_moves[i] = move_value;
            nb_moves_made++;

        }

        // undoing the moves made
        for (int i = 0; i < nb_moves_made; i++){
            undo_last_move_AI();
        }

        // adding the line to the opening book only if it's complete
        if (is_line_valid && nb_moves_made == NUMBER_OF_OPENING_MOVES){
            if (nb_opening_book_lines == capacity){
                capacity = (capacity == 0) ? 1024 : 2*capacity;
                void* new_lines = realloc(opening_book_lines, capacity*sizeof(*opening_book_lines));
                // looking for malloc errors
                if (new_lines == NULL){
                    fprintf(stderr, "Error: memory allocation failed in load_opening_book\n");
                    break;
                }
                opening_book_lines = new_lines;
            }
            memcpy(opening_book_lines[nb_opening_book_lines], line_moves, sizeof(line_moves));
            nb_opening_book_lines++;
        }
    }

    // closing the file
    fclose(formal_file);
    if (nb_skipped_lines > 0){
        fprintf(stderr, "Opening book : %d lines skipped because a move wasn't understood\n", nb_skipped_lines);
    }

    // reset everything 
    reset_bitboards();
    reset_log();
    
}


bool is_move_in_move_array(move move_value, const move* moves_array, int nb_moves){

    for (int i = 0; i < nb_moves; i++){
        if (moves_array[i] == move_value){
            return true;
        }
    }
    return false;

}


move get_move_from_book(){

    // if the book isn't used, or if we're too far in the game, we need to stop the search in the opening book, because we want to test the AI, not to replicate games
    if (!is_opening_book_enabled || opening_book_lines == NULL || chess_log->actual_size >= opening_book_max_plies){
        return 0;
    }

    // the different moves that the book proposes in this position
    move moves_possibles_from_book[256];
    int nb_moves = 0;
    // going through the lines of the opening book, to keep the ones that started like the current game
    for (int line = 0; line < nb_opening_book_lines; line++){
        int i = 0;
        while (i < chess_log->actual_size && opening_book_lines[line][i] == chess_log->moves[i]->move_value){
            i++;
        }
        if (i == chess_log->actual_size){
            move move_possible = opening_book_lines[line][chess_log->actual_size];
            if (nb_moves < 256 && !is_move_in_move_array(move_possible, moves_possibles_from_book, nb_moves)){
                moves_possibles_from_book[nb_moves] = move_possible;
                nb_moves++;
            }
        }
    }

    printf("nb_moves : %d\n", nb_moves);
    for (int i = 0; i < nb_moves; i++){
        printf("move %d : %d\n", i, moves_possibles_from_book[i]);
    }
    printf("\n");

    if (nb_moves == 0){
        return 0;
    }
    set_seed();
    return moves_possibles_from_book[random_int(0, nb_moves - 1)];

}
