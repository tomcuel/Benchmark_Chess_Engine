#ifndef __AI_H___
#define __AI_H___


#include "SDL2/SDL.h"


#include "../Alpha_Beta/Openings.h"
#include "../Alpha_Beta/Search.h"
#include "../Game_Treatment/Board.h"
#include "../Game_Treatment/Log.h"
#include "../Game_Treatment/Moves_Treatment.h"
#include "../Game_Treatment/Settings.h"
#include "../Useful/Constants.h"
#include "../Useful/Utility.h"


/////////////////////////////////////////////////////////////////////////////////////
// AI summary
// This part of the code assemble all the functions that are related to the AI testings to create a function that make the AI play
// It also contains the principal function to make the AI play on top of all the search testing functions
/**
 * A summary of the supported function is given below:
 * 
 * make_AI_play : Function to make the AI play (also the final level of the AI entirely)
 * run_best_AI_level : Function to make the AI play at the best level possible (also including opening book)
**/
/////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////
// Function to make the AI play at the best level possible (also including opening book)
/**
 * @param selected_move : the selected move to play
 * @param selected_score : the score of the selected move
 * @param is_move_taken_from_book : boolean to know if the move is taken from the opening book
 * @param depth_reached : the depth reached in the search
**/
/////////////////////////////////////////////////////////////////////////////////////
void run_best_AI_level(move* selected_move, int* selected_score, bool* is_move_taken_from_book, int* depth_reached);


/////////////////////////////////////////////////////////////////////////////////////
// Function to make the AI play (also the final level of the AI entirely)
/**
 * @param selected_move : the selected move to play
 * @param selected_score : the score of the selected move
 * @param is_move_taken_from_book : boolean to know if the move is taken from the opening book
 * @param depth_reached : the depth reached in the search
**/
/////////////////////////////////////////////////////////////////////////////////////
void make_AI_play(move* selected_move, int* selected_score, bool* is_move_taken_from_book, int* depth_reached);


#endif /* __AI_H__ */