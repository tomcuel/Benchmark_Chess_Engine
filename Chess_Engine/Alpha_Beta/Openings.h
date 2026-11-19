#ifndef __OPENINGS_H__
#define __OPENINGS_H__


#include "../Game_Treatment/Board.h"
#include "../Game_Treatment/Log.h"
#include "../Game_Treatment/Move_Generator.h"
#include "../Game_Treatment/Settings.h"
#include "../Useful/Constants.h"


/////////////////////////////////////////////////////////////////////////////////////
// Openings summary
// This module is responsible for the generation and management of the opening book
/**
 * A summary of the supported functions is given below:
 * 
 * set_opening_book_path : Function to choose the file from which the opening book is loaded
 * set_opening_book_usage : Function to enable or disable the opening book, and to choose how many moves can be taken from it
 * get_number_of_opening_book_lines : Function to get the number of lines loaded in the opening book
 * load_opening_book : Function to load the opening book of all the games in memory, it created the opening book not with values but with move_value structures
 * free_opening_book : Function to free the opening book
 * is_move_in_move_array : Function to check if a move is in an array of moves
 * get_move_from_book : Function to get the move from the opening book
**/
/////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////
// Function to choose the file from which the opening book is loaded (to call before load_opening_book)
// by default it's "Chess_Engine/Alpha_Beta/Data/Opening_in_Games.txt", relative to where the program is launched
/**
 * @param path : the path of the file with the openings in formal chess language
**/
/////////////////////////////////////////////////////////////////////////////////////
void set_opening_book_path(const char* path);


/////////////////////////////////////////////////////////////////////////////////////
// Function to enable or disable the opening book, and to choose how many moves can be taken from it
/**
 * @param is_enabled : true to use the opening book, false otherwise
 * @param max_plies : the number of half-moves of the game during which the book can be used (at most NUMBER_OF_OPENING_MOVES)
**/
/////////////////////////////////////////////////////////////////////////////////////
void set_opening_book_usage(bool is_enabled, int max_plies);


/////////////////////////////////////////////////////////////////////////////////////
// Function to get the number of lines loaded in the opening book
/**
 * @return the number of lines of the opening book (0 if it's not loaded)
**/
/////////////////////////////////////////////////////////////////////////////////////
int get_number_of_opening_book_lines();


/////////////////////////////////////////////////////////////////////////////////////
// Function to load the opening book of all the games in memory, it created the opening book not with values but with move_value structures
// the book is only read, the current game is compared to it each time a move is asked, so there is nothing to update during the game
/////////////////////////////////////////////////////////////////////////////////////
void load_opening_book();


/////////////////////////////////////////////////////////////////////////////////////
// Function to free the opening book
/////////////////////////////////////////////////////////////////////////////////////
void free_opening_book();


/////////////////////////////////////////////////////////////////////////////////////
// Function to check if a move is in an array of moves
/**
 * @param move_value : the move to check
 * @param moves_array : the array of moves
 * @param nb_moves : the number of moves in the array
 * @return true if the move is in the array, false otherwise
**/
/////////////////////////////////////////////////////////////////////////////////////
bool is_move_in_move_array(move move_value, const move* moves_array, int nb_moves);


/////////////////////////////////////////////////////////////////////////////////////
// Function to get the move from the opening book
/**
 * @return the move from the opening book if it's in the book, else 0
**/
/////////////////////////////////////////////////////////////////////////////////////
move get_move_from_book();


#endif /* __OPENINGS_H__ */