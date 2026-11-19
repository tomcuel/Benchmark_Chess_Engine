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
// Time given to the search in milliseconds, when it needs to be imposed (by a benchmark for example)
// when it's 0 (the default value), the time is chosen with the clock of the game
/////////////////////////////////////////////////////////////////////////////////////
extern int forced_think_time_in_ms;


/////////////////////////////////////////////////////////////////////////////////////
// Maximum depth of the iterative deepening, when it needs to be imposed
// when it's 0 (the default value), the search only stops with the time
/////////////////////////////////////////////////////////////////////////////////////
extern int forced_max_depth;


/////////////////////////////////////////////////////////////////////////////////////
// To stop the search from outside of it (from another thread), it stops as soon as possible and gives the best move found until then
// it needs to be put back to false before starting a new search
/////////////////////////////////////////////////////////////////////////////////////
extern volatile bool is_search_stop_requested;


/////////////////////////////////////////////////////////////////////////////////////
// Function called each time a depth of the iterative deepening is completed (NULL by default, nothing is called)
/**
 * @param depth : the depth that has just been completed
 * @param score : the score of the best move found at this depth
 * @param best_move_found : the best move found at this depth
**/
/////////////////////////////////////////////////////////////////////////////////////
extern void (*search_iteration_callback)(int depth, int score, move best_move_found);


/////////////////////////////////////////////////////////////////////////////////////
// Structure with the counters of a search, to measure how the search behaves (they are reset at the start of each search)
/////////////////////////////////////////////////////////////////////////////////////
typedef struct Search_Statistics {
    long long nodes; // the number of positions visited (in the main search and in the quiescence search)
    long long quiescence_nodes; // the part of them visited in the quiescence search
    long long transposition_hits; // the number of positions whose value was found in the transposition table
    long long beta_cutoffs; // the number of positions where a move was good enough to stop looking at the other ones (alpha-beta cutoff)
    long long first_move_beta_cutoffs; // the part of them where it was the first move tried (shows the quality of the move ordering)
} Search_Statistics;
extern Search_Statistics search_statistics;


/////////////////////////////////////////////////////////////////////////////////////
// To see the draws by repetition of position and by the 50 moves rule in the search (true by default)
// without it, the AI can repeat a position in a winning game and let the opponent claim the draw
/////////////////////////////////////////////////////////////////////////////////////
extern bool is_draw_detection_enabled;


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