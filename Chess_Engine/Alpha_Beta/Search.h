#ifndef __SEARCH_H__
#define __SEARCH_H__


#include "Evaluation.h"
#include "../Game_Treatment/Move_Generator.h"
#include "Move_Ordering.h"
#include "Openings.h"
#include "Transposition_Table.h"
#include "../Useful/Constants.h"


/////////////////////////////////////////////////////////////////////////////////////
// Search summary
// This module is responsible for the search algorithms of the level 7 AI, to find the best move to play (iterative deepening, alpha-beta pruning, quiescence search, transposition tables, opening book)
/**
 * A summary of the supported functions is given below:
 * 
 * choose_think_time : Function to choose the best think for this state of the game
 * run_iterative_deepening : Function to make search for the move by using the iterative deepening method for a given time of search
 * Search : Function to make search for the best evaluation of a move with the alpha-beta pruning with a more complex evaluation of the position, considering depth extensions for critical moves, but with an extended search for the captures after the depth is reached with the transposition tables to fasten the search
 * Quiescience_Search : Function to make search for the best evaluation of a move with the alpha-beta pruning with a more complex evaluation of the position, only for the captures without depth restriction
**/
/////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////
// Function to choose the best think for this state of the game
/**
 * @param color_playing : the color of the player that is playing
 * @return the think time for the AI
**/
/////////////////////////////////////////////////////////////////////////////////////
int choose_think_time(int color_playing);


/////////////////////////////////////////////////////////////////////////////////////
// Function to make search for the move by using the iterative deepening method for a given time of search
/**
 * @param selected_move : the selected move to play
 * @param selected_score : the score of the selected move
 * @param depth_reached : the depth reached in the search
**/
/////////////////////////////////////////////////////////////////////////////////////
void run_iterative_deepening(move* selected_move, int* selected_score, int* depth_reached);


/////////////////////////////////////////////////////////////////////////////////////
// Function to make search for the best evaluation of a move with the alpha-beta pruning with a more complex evaluation of the position, considering depth extensions for critical moves, but with an extended search for the captures after the depth is reached with the transposition tables to fasten the search
// some conditions of time for the iterative deepening were added
/**
 * @param ply_remaining : the number of ply remaining to reach the depth we need to reach in the alpha-beta pruning search 
 * @param ply_from_root : the number of ply from the root of the search (depth currently reached)
 * @param alpha : the alpha value
 * @param beta : the beta value
 * @param nb_extensions : the number of extensions to add to the search
 * @return the score of the best evaluation of a move
**/
/////////////////////////////////////////////////////////////////////////////////////
int Search(int ply_remaining, int ply_from_root, int alpha, int beta, int nb_extensions);


/////////////////////////////////////////////////////////////////////////////////////
// Function to make search for the best evaluation of a move with the alpha-beta pruning with a more complex evaluation of the position, only for the captures without depth restriction
// some conditions of time for the iterative deepening were added
/**
 * @param alpha : the alpha value
 * @param beta : the beta value
 * @return the score of the best evaluation of a move
**/
/////////////////////////////////////////////////////////////////////////////////////
int Quiescience_Search(int alpha, int beta);


#endif /* __SEARCH_H__ */