#ifndef __UTILITY_H__
#define __UTILITY_H__


#include "Constants.h"


/////////////////////////////////////////////////////////////////////////////////////
// Useful summary
// This module is responsible for creating all the useful function that are used in the game but not part of the game
// It only depends on the standard library
/**
 * A summary of the supported functions is given below:
 * 
 * get_time_ms: Function to get the time elapsed in milliseconds (monotonic clock)
 * 
 * min: Function that returns the minimum of two integers
 * max: Function that returns the maximum of two integers
 * 
 * set_seed: Function to set a custom seed based on current time (unless a fixed seed has been given)
 * set_fixed_seed: Function to set a given seed, to be able to replay the same random choices
 * custom_rand: Function to generate a random number (LCG)
 * random_int: Function to generate a random number within a given range
 * 
 * int_clamp: Function to clamp a value between a minimum and a maximum (for integers)
 * float_clamp: Function to clamp a value between a minimum and a maximum (for floats)
 * get_rank: Function to get the rank of a square of the chess board
 * get_file: Function to get the file of a square of the chess board
 * get_index: Function to get the index of a square of the chess board
 * divide_by_two: Function to divide a number by two
**/
/////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////
// Function to get the time elapsed in milliseconds since an arbitrary point (monotonic clock)
// used to time the AI search
/**
 * @return the time in milliseconds
**/
/////////////////////////////////////////////////////////////////////////////////////
int get_time_ms();


/////////////////////////////////////////////////////////////////////////////////////
// Function that returns the minimum of two integers
/**
 * @param a : the first integer
 * @param b : the second integer
 * @return the minimum of the two integers
**/
/////////////////////////////////////////////////////////////////////////////////////
int min(int a, int b);


/////////////////////////////////////////////////////////////////////////////////////
// Function that returns the maximum of two integers
/**
 * @param a : the first integer
 * @param b : the second integer
 * @return the maximum of the two integers
**/
/////////////////////////////////////////////////////////////////////////////////////
int max(int a, int b);


/////////////////////////////////////////////////////////////////////////////////////
// Function to set a custom seed based on current time
// it does nothing if a fixed seed has been given with set_fixed_seed
/////////////////////////////////////////////////////////////////////////////////////
void set_seed();


/////////////////////////////////////////////////////////////////////////////////////
// Function to set a given seed, the following calls to set_seed will then be ignored
// used to be able to replay the same random choices (in the benchmarks for example)
/**
 * @param value : the seed to use
**/
/////////////////////////////////////////////////////////////////////////////////////
void set_fixed_seed(unsigned int value);


/////////////////////////////////////////////////////////////////////////////////////
// Function to generate a random number (LCG)
/**
 * @return the random number
**/
/////////////////////////////////////////////////////////////////////////////////////
unsigned int custom_rand();


/////////////////////////////////////////////////////////////////////////////////////
// Function to generate a random integer between min and max
/**
 * @param min : the minimum value
 * @param max : the maximum value
**/
/////////////////////////////////////////////////////////////////////////////////////
int random_int(int min, int max);


/////////////////////////////////////////////////////////////////////////////////////
// Function to clamp a value between a minimum and a maximum (for integers)
/**
 * @param value : the value to clamp
 * @param min : the minimum value
 * @param max : the maximum value
 * @return the clamped value
**/
/////////////////////////////////////////////////////////////////////////////////////
int int_clamp(int value, int min, int max);


/////////////////////////////////////////////////////////////////////////////////////
// Function to clamp a value between a minimum and a maximum (for floats)
/**
 * @param value : the value to clamp
 * @param min : the minimum value
 * @param max : the maximum value
 * @return the clamped value
**/
/////////////////////////////////////////////////////////////////////////////////////
float float_clamp(float value, float min, float max);


/////////////////////////////////////////////////////////////////////////////////////
// Function to get the rank of a square of the chess board
// to fast things in the AI search part
/**
 * @param square : the square to get the rank of
 * @return the rank of the square
**/
/////////////////////////////////////////////////////////////////////////////////////
int get_rank(int square);


/////////////////////////////////////////////////////////////////////////////////////
// Function to get the file of a square of the chess board
// to fast things in the AI search part
/**
 * @param square : the square to get the file of
 * @return the file of the square
**/
/////////////////////////////////////////////////////////////////////////////////////
int get_file(int square);


/////////////////////////////////////////////////////////////////////////////////////
// Function to get the index of a square of the chess board
// to fast things in the AI search part
/**
 * @param rank : the rank of the square
 * @param file : the file of the square
 * @return the index of the square
**/
/////////////////////////////////////////////////////////////////////////////////////
int get_index(int rank, int file);


/////////////////////////////////////////////////////////////////////////////////////
// Function to divide a number by two
// to fast things in the graphics printing part
/**
 * @param number : the number to divide by two
 * @return the number divided by two
**/
/////////////////////////////////////////////////////////////////////////////////////
int divide_by_two(int number);


#endif /* __UTILITY_H__ */