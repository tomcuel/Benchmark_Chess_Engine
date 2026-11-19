#include "Utility.h"


int get_time_ms() {

    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    static time_t first_second = 0; // to start near 0 and avoid any overflow of the int
    if (first_second == 0) {
        first_second = ts.tv_sec;
    }
    return (int)((ts.tv_sec - first_second) * 1000 + ts.tv_nsec / 1000000);

}


int min(int a, int b) {
    
    if (a < b) {
        return a;
    }
    return b;

}


int max(int a, int b) {

    if (a > b) {
        return a;
    }
    return b;

}


static unsigned int seed;  // Initialize without a specific seed
static const unsigned int a = 1664525;
static const unsigned int long c = 1013904223;
static const unsigned int long m = 4294967296;  // 2^32


static bool is_seed_fixed = false; // true when a seed has been given with set_fixed_seed


void set_seed() {

    // a fixed seed has been given, we keep the random sequence going from it
    if (is_seed_fixed) {
        return;
    }
    // Use current time to initialize seed
    seed = (unsigned int)(time(NULL) ^ getpid());

}


void set_fixed_seed(unsigned int value) {

    seed = value;
    is_seed_fixed = true;

}


unsigned int custom_rand() {
    
    seed = (a * seed + c) % m;  // Update seed using LCG formula
    return seed;

}


int random_int(int min, int max) {
    
    return min + custom_rand() % (max - min + 1);
    
}


int int_clamp(int value, int min, int max) {
    
    if (value < min) return min;
    if (value > max) return max;
    return value;

}


float float_clamp(float value, float min, float max) {
    
    if (value < min) return min;
    if (value > max) return max;
    return value;

}


int get_rank(int square) {
    
    return square >> 3; // could be written as square / 8, using the bit shift operator is faster

}


int get_file(int square) {
    
    return square & 0x07; // could be written as square % 8, using the bitwise AND operator is faster

}


int get_index(int rank, int file) {
    
    return (rank << 3) | file; // could be written as rank * 8 + file, using the bit shift operator is faster

}


int divide_by_two(int number) {
    
    return number >> 1;  // could be written as number / 2, using the bit shift operator is faster

}
