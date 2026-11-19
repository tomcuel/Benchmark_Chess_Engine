#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__


#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h> 


/////////////////////////////////////////////////////////////////////////////////////
// Constants for the chessboard
/////////////////////////////////////////////////////////////////////////////////////
#define BOARD_SIZE 8 // size of the chessboard (8x8)
#define NUMBER_SQUARES 64 // number of squares on the chessboard


/////////////////////////////////////////////////////////////////////////////////////
// Game Settings
/////////////////////////////////////////////////////////////////////////////////////
// Defining the two players (Undetermined when none of them is ahead)
enum {White_Player, Black_Player, Undetermined};


/////////////////////////////////////////////////////////////////////////////////////
// Memory related constants
/////////////////////////////////////////////////////////////////////////////////////
// Define the size of the log array
#define max_size_log_array 2048 // number of half-moves that can be stored : a long game against another engine can go over 600, and the moves tried by the search are added on top of it
// Define the size of the valid moves array
#define max_size_valid_moves_array 100
#define max_size_valid_moves_array_extended 218
// 1024 (16x64) moves maximum should the maximum number of moves possible in a game for a color without obstacles and rules
// 218 is the theoretical maximum number of moves possible in a game for a color
// 100 is most of the time a number of moves never reached in a game for a color, so it should be enough and fast things up
// we can allocate more memory if needed with the 218 if we encouter this situation, or one with more than 100 moves


/////////////////////////////////////////////////////////////////////////////////////
// Bitboards related constants
/////////////////////////////////////////////////////////////////////////////////////
// Enum that represents the type of the pieces (to be able to go through the bitboard array)
#define NUMBER_OF_BIBOARD 14
#define number_of_bitboards_per_color 7
// Define the number of piece of each color that can be captured for each color
#define NUMBER_OF_PIECES_TAKENABLE_PER_COLOR 15
// Enum that represents the two colors of the pieces 
enum {WHITE, BLACK, NO_COLOR};
enum {
    PAWN_WHITE /* 0 */,
    KNIGHT_WHITE /* 1 */, 
    BISHOP_WHITE, /* 2 */
    ROOK_WHITE, /* 3 */
    QUEEN_WHITE, /* 4 */
    KING_WHITE, /* 5 */
    WHITE_PIECES, /* 6 */
    PAWN_BLACK, /* 7 */
    KNIGHT_BLACK, /* 8 */
    BISHOP_BLACK, /* 9 */
    ROOK_BLACK, /* 10 */
    QUEEN_BLACK, /* 11 */
    KING_BLACK, /* 12 */
    BLACK_PIECES, /* 13 */
    NOTHING /* 14 */
};
// Define what a board is (a 64bit integer)
typedef uint64_t bit_board;
// Define what a move is (a 16bit integer) -> bits 0-5 : start square, bits 6-11 : target square, bits 12-15 : flags
typedef uint16_t move;
// Define what a castling rights is (a 8bit integer) -> bit 0 : white king moved, bit 1 : white rook A moved, bit 2 : white rook H moved, bit 3 : black king moved, bit 4 : black rook A moved, bit 5 : black rook H moved
typedef uint8_t castling_rights_flags;
// Enum that represents the check state 
enum {NO_CHECK, CHECK, WHITE_CHECK, BLACK_CHECK, CHECK_MATE, NO_DRAW, STALEMATE, DRAW_BY_3_RULES, DRAW_BY_50_RULES, DRAW_BY_INSUFFICIENT_MATERIAL};


/////////////////////////////////////////////////////////////////////////////////////
// Flags and masks constants
/////////////////////////////////////////////////////////////////////////////////////
// Masks (declaration but no initialization)
#define start_square_mask 0b0000000000111111
#define target_square_mask 0b0000111111000000
#define flag_mask 0b1111000000000000
#define File_A_mask 0x0101010101010101ULL
#define Full_Bitboard_mask 0xFFFFFFFFFFFFFFFFULL
#define Empty_Bitboard_mask 0x0000000000000000ULL
// Flags (declaration but no initialization)
#define No_Flag 0b0000
#define En_Passant_Flag 0b0001
#define Castle_Flag 0b0010
#define Pawn_Two_Up_Flag 0b0011
#define Promote_To_Queen_Flag 0b0100
#define Promote_To_Knight_Flag 0b0101
#define Promote_To_Rook_Flag 0b0110
#define Promote_To_Bishop_Flag 0b0111
// Castling rights flags (declaration but no initialization)
#define All_Castling_Rights 0b000000
#define White_King_Moved_Flag 0b000001
#define White_Rook_A_Moved_Flag 0b000010
#define White_Rook_H_Moved_Flag 0b000100
#define Black_King_Moved_Flag 0b001000
#define Black_Rook_A_Moved_Flag 0b010000
#define Black_Rook_H_Moved_Flag 0b100000


/////////////////////////////////////////////////////////////////////////////////////
// Constants for the AI related part
/////////////////////////////////////////////////////////////////////////////////////
// Constants to define the value of each piece
#define PAWN_VALUE 100
#define KNIGHT_VALUE 300
#define BISHOP_VALUE 320
#define ROOK_VALUE 500
#define QUEEN_VALUE 900
#define KING_VALUE 1200
// Constants to define the maximum score of the IA (or the minimum score of the IA depending on the case)(for easier levels)
#define MAX_IA_SCORE 100000
// Constants to define the max score of the AI for the more advanced levels
#define IMMEDIATE_MATE_SCORE 100000
#define POSITIVE_INFINITY 9999999
#define NEGATIVE_INFINITY -9999999
// Constants to define the flags linked to the type of cut in the alpha-beta pruning has been made (for the transposition tables)
enum{EXACT, ALPHA_CUT, BETA_CUT};
// Constant to define the value for a failed lookup in the transposition tables
#define FAILED_LOOKUP -1
// Constants to define the maximum number of extensions for the depth of the search of the IA
#define MAX_NUMBER_OF_EXTENSIONS 16
// Constants to define the maximum depth in the iterative deepening search
#define MAX_DEPTH_ITERATIVE_DEEPENING 100
// Constants to define the number of pre computed moves for the opening book (10 means 5 complete moves) (can be changed to have more or less opening moves)
#define NUMBER_OF_OPENING_MOVES 12


#endif /* __CONSTANTS_H__ */