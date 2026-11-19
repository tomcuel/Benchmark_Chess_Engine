#ifndef __FEN_H__
#define __FEN_H__


#include "Board.h"
#include "Log.h"
#include "Moves_Treatment.h"
#include "Settings.h"
#include "../Useful/Constants.h"
#include "../Useful/Utility.h"


/////////////////////////////////////////////////////////////////////////////////////
// Fen summary
// This module is responsible for setting up the board from a position
// A position can be given in the FEN notation (Forsyth-Edwards Notation), ex for the start of the game :
// rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
/**
 * A summary of the supported functions is given below:
 * 
 * set_start_position : Function to put the board in the starting position of a game, with an empty log
 * set_position_from_fen : Function to put the board in the position described by a FEN
**/
/////////////////////////////////////////////////////////////////////////////////////


// The FEN of the starting position of a game
#define START_POSITION_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"


/////////////////////////////////////////////////////////////////////////////////////
// Function to put the board in the starting position of a game, with an empty log (white is playing)
// the log being empty, the opening book can be used
/////////////////////////////////////////////////////////////////////////////////////
void set_start_position();


/////////////////////////////////////////////////////////////////////////////////////
// Function to put the board in the position described by a FEN
// The castling rights and the en passant square are kept in the log of the game, so the log starts with one element
// that doesn't correspond to a move really played : it holds the castling rights, and the pawn move of two squares
// that allows the en passant (if there is one). This element must never be undone.
// The number of half-moves for the 50 moves rule is kept in log_setup_halfmove_clock, the last field (number of the move) is ignored.
/**
 * @param fen : the position in the FEN notation
 * @return true if the position has been set, false if the FEN isn't valid (the board is then in the starting position)
**/
/////////////////////////////////////////////////////////////////////////////////////
bool set_position_from_fen(const char* fen);


#endif /* __FEN_H__ */
