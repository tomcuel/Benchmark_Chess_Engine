#ifndef __MOVE_GENERATOR_H__
#define __MOVE_GENERATOR_H__


#include "Board.h"
#include "Log.h"
#include "Moves_Treatment.h"
#include "Settings.h"
#include "../Useful/Constants.h"


/////////////////////////////////////////////////////////////////////////////////////
// Move_Generator summary
// This module is used to generate all the possible moves, the state of the game and the IA making and undoing moves function
// it's also a function to get the check state of the game
/**
 * A summary of the supported functions is given below:
 * 
 * are_they_possible_moves : Function to check if there is a possible move for the player
 * get_check_state : Function to get the check state of the game
 * get_game_result : Function to get the result of the game depending of the situation (for the RL engine)
 * is_game_over : Function to to state if the game if finished
 * 
 * generate_all_possible_moves : Function to generate all the possible moves for the player (here we care if we are on check after the move)
 * generate_all_possible_moves_that_capture : Function to generate all the possible moves that capture a piece for the player (here we care if we are on check after the move)
 *
 * make_move_AI : Function to make a move by supposing that the move can be done (will be used in search algorithms of the AI)
 * undo_last_move_AI : Function to undo the last move done by the AI (without the captured pieces and the score as we don't care about them in the search algorithms, as stated in the make_move_AI function)
**/
/////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////
// Function to check if there is a possible move for the player
/** 
 * @return true if there is a possible move for the player, false otherwise
**/
/////////////////////////////////////////////////////////////////////////////////////
bool are_they_possible_moves();


/////////////////////////////////////////////////////////////////////////////////////
// Function to get the check state of the game
/**
 * @param color : the color of the player that is playing
 * @return the check state of the game (NO_CHECK, CHECK, CHECK_MATE, DRAW)
**/
/////////////////////////////////////////////////////////////////////////////////////
int get_check_state(int color);


/////////////////////////////////////////////////////////////////////////////////////
// Function to get the result of the game depending of the situation (for the RL engine)
/**
 * @param color : the color of the player that is playing
 * @return the check state of the game (-1, 0, +1, 2)
**/
/////////////////////////////////////////////////////////////////////////////////////
int get_game_result(int color);


/////////////////////////////////////////////////////////////////////////////////////
// Function to to state if the game if finished 
/**
 * @param color : the color of the player that is playing
 * @return if the game is finished (0, 1)
**/
/////////////////////////////////////////////////////////////////////////////////////
int is_game_over(int color);


/////////////////////////////////////////////////////////////////////////////////////
// Function to generate all the possible moves for the player (here we care if we are on check after the move)
/** 
 * @param nb_moves : The number of valid moves, will be modified by the function, as a pointer
 * @return an array of moves (containing the from and to square of the move, but also the flags of the move)
**/
/////////////////////////////////////////////////////////////////////////////////////
move* generate_all_possible_moves(int* nb_moves);


/////////////////////////////////////////////////////////////////////////////////////
// Function to generate all the possible moves that capture a piece for the player (here we care if we are on check after the move)
/** 
 * @param nb_moves : The number of valid moves, will be modified by the function, as a pointer
 * @return an array of moves (containing the from and to square of the move, but also the flags of the move)
**/
/////////////////////////////////////////////////////////////////////////////////////
move* generate_all_possible_moves_that_capture(int* nb_moves);


/////////////////////////////////////////////////////////////////////////////////////
// Function to make a move by supposing that the move can be done (will be used in search algorithms of the AI)
// we don't use the captured pieces and the score because we don't care about them in the search algorithms, it's just to simulate the move and the captured pieces and score is only used to show information to the player
/**
 * @param move_value : The move to do (containing the from and to square of the move, but also the flags of the move, useful so we don't have to calculate them again)
**/
/////////////////////////////////////////////////////////////////////////////////////
void make_move_AI(move move_value);


/////////////////////////////////////////////////////////////////////////////////////
// Function to undo the last move done by the AI (without the captured pieces and the score as we don't care about them in the search algorithms, as stated in the make_move_AI function)
/////////////////////////////////////////////////////////////////////////////////////
void undo_last_move_AI();


/////////////////////////////////////////////////////////////////////////////////////
// To choose if the promotions to a rook, a bishop and a knight are generated with the promotion to a queen
// false by default : the AI only looks at the promotion to a queen. It's set to true to check the move generation (perft)
/////////////////////////////////////////////////////////////////////////////////////
extern bool are_under_promotions_generated;


#endif /* __MOVE_GENERATOR_H__ */