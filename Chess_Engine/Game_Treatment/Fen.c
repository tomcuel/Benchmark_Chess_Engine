#include "Fen.h"


// Function to reset what is common to every new position : the logs, the captured pieces and the player that is playing
static void reset_game_state(int color_playing) {

    reset_log();
    reset_bitboards_log();
    reset_captured_pieces_and_score();
    game_settings->color_of_player_that_is_playing = color_playing;
    game_settings->player_that_is_playing = (color_playing == WHITE) ? White_Player : Black_Player;

}


void set_start_position() {

    reset_bitboards();
    reset_game_state(WHITE);

}


// Function to get the type of piece corresponding to a letter of the FEN (NOTHING if the letter isn't a piece)
static int get_piece_type_from_fen_letter(char letter) {

    switch (letter) {
        case 'P': return PAWN_WHITE;
        case 'N': return KNIGHT_WHITE;
        case 'B': return BISHOP_WHITE;
        case 'R': return ROOK_WHITE;
        case 'Q': return QUEEN_WHITE;
        case 'K': return KING_WHITE;
        case 'p': return PAWN_BLACK;
        case 'n': return KNIGHT_BLACK;
        case 'b': return BISHOP_BLACK;
        case 'r': return ROOK_BLACK;
        case 'q': return QUEEN_BLACK;
        case 'k': return KING_BLACK;
        default: return NOTHING;
    }

}


bool set_position_from_fen(const char* fen) {

    // the different fields of the FEN (the two last ones are optional and not used)
    char pieces_field[100] = "";
    char color_field[4] = "";
    char castling_field[8] = "-";
    char en_passant_field[4] = "-";
    int halfmove_clock = 0;
    if (fen == NULL || sscanf(fen, "%99s %3s %7s %3s %d", pieces_field, color_field, castling_field, en_passant_field, &halfmove_clock) < 2) {
        set_start_position();
        return false;
    }

    // reading the pieces in temporary bitboards, the FEN goes from the rank 8 to the rank 1, and from the file a to the file h
    bit_board new_bitboards[NUMBER_OF_BIBOARD] = {0};
    int rank = 7;
    int file = 0;
    for (int i = 0; pieces_field[i] != '\0'; i++) {
        char letter = pieces_field[i];
        if (letter == '/') {
            // each rank must be complete before going to the next one
            if (file != 8 || rank == 0) {
                set_start_position();
                return false;
            }
            rank--;
            file = 0;
        }
        else if (letter >= '1' && letter <= '8') {
            file += letter - '0';
        }
        else {
            int piece_type = get_piece_type_from_fen_letter(letter);
            if (piece_type == NOTHING || file > 7) {
                set_start_position();
                return false;
            }
            new_bitboards[piece_type] |= (1ULL << get_index(rank, file));
            new_bitboards[(piece_type < WHITE_PIECES) ? WHITE_PIECES : BLACK_PIECES] |= (1ULL << get_index(rank, file));
            file++;
        }
        if (file > 8) {
            set_start_position();
            return false;
        }
    }
    // the 8 ranks must have been given, and there must be exactly one king of each color
    if (rank != 0 || file != 8 || __builtin_popcountll(new_bitboards[KING_WHITE]) != 1 || __builtin_popcountll(new_bitboards[KING_BLACK]) != 1) {
        set_start_position();
        return false;
    }

    // the color that is playing
    int color_playing = NO_COLOR;
    if (strcmp(color_field, "w") == 0) {
        color_playing = WHITE;
    }
    else if (strcmp(color_field, "b") == 0) {
        color_playing = BLACK;
    }
    else {
        set_start_position();
        return false;
    }

    // the castling rights : in the log, a flag is set when a castle isn't possible anymore because the king or the rook has moved
    castling_rights_flags castling_rights = All_Castling_Rights;
    bool can_white_castle_short = (strchr(castling_field, 'K') != NULL);
    bool can_white_castle_long = (strchr(castling_field, 'Q') != NULL);
    bool can_black_castle_short = (strchr(castling_field, 'k') != NULL);
    bool can_black_castle_long = (strchr(castling_field, 'q') != NULL);
    if (!can_white_castle_short) {
        set_castling_flag(&castling_rights, White_Rook_H_Moved_Flag);
    }
    if (!can_white_castle_long) {
        set_castling_flag(&castling_rights, White_Rook_A_Moved_Flag);
    }
    if (!can_white_castle_short && !can_white_castle_long) {
        set_castling_flag(&castling_rights, White_King_Moved_Flag);
    }
    if (!can_black_castle_short) {
        set_castling_flag(&castling_rights, Black_Rook_H_Moved_Flag);
    }
    if (!can_black_castle_long) {
        set_castling_flag(&castling_rights, Black_Rook_A_Moved_Flag);
    }
    if (!can_black_castle_short && !can_black_castle_long) {
        set_castling_flag(&castling_rights, Black_King_Moved_Flag);
    }

    // the en passant square : it's the square behind the pawn that just moved two squares
    // in the log, the en passant is found with the last move, so we rebuild the move of the pawn
    move previous_move = 0;
    if (strcmp(en_passant_field, "-") != 0) {
        int en_passant_file = en_passant_field[0] - 'a';
        int en_passant_rank = en_passant_field[1] - '1';
        // the square must be on the rank 6 when white is playing (a black pawn just moved), on the rank 3 when black is playing
        if (en_passant_file < 0 || en_passant_file > 7 || en_passant_rank != ((color_playing == WHITE) ? 5 : 2)) {
            set_start_position();
            return false;
        }
        int en_passant_square = get_index(en_passant_rank, en_passant_file);
        int pawn_from = (color_playing == WHITE) ? en_passant_square + 8 : en_passant_square - 8;
        int pawn_to = (color_playing == WHITE) ? en_passant_square - 8 : en_passant_square + 8;
        // the pawn that moved must be there, otherwise the en passant square is simply ignored
        if (new_bitboards[(color_playing == WHITE) ? PAWN_BLACK : PAWN_WHITE] & (1ULL << pawn_to)) {
            previous_move = get_move_value(pawn_from, pawn_to, Pawn_Two_Up_Flag);
        }
    }

    // everything is valid, we can now set the position
    for (int i = 0; i < NUMBER_OF_BIBOARD; i++) {
        bitboards[i] = new_bitboards[i];
    }
    reset_game_state(color_playing);
    // the first element of the log holds the castling rights and the en passant possibility of the position
    chess_log->moves[0]->move_value = previous_move;
    chess_log->moves[0]->captured_piece_type = NOTHING;
    chess_log->moves[0]->castling_rights = castling_rights;
    chess_log->actual_size = 1;
    log_setup_size = 1;
    log_setup_halfmove_clock = int_clamp(halfmove_clock, 0, 100);

    return true;

}
