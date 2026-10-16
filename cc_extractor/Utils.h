#ifndef UTILS_H
#define UTILS_H

#include <bitset>
#include <cassert>
#include <iostream>
#include <vector>
#include <stack>
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <queue>
//#include "boinc_api.h"

typedef unsigned __int128 uint128_t;

extern int minRows[6];
extern int maxRows[6];

extern std::bitset<36> rowBits[]; //Pre-computed rows.
extern std::bitset<36> rowBitsOneTwo[]; //Pre-computed 2x1 pieces in each row.
extern std::bitset<36> rowBitsOneThree[]; //Pre-computed 3x1 pieces in each row.
extern std::bitset<36> colBits[22]; // Same as above but for columns.
extern std::bitset<36> colBitsOneTwo[22];
extern std::bitset<36> colBitsOneThree[22];

extern int boardSizes[2];
extern std::vector<char> piecesThreebyOneHorizontal;
extern std::vector<char> piecesThreebyOneVertical;
extern std::vector<char> piecesTwobyOneHorizontal;
extern std::vector<char> piecesTwobyOneVertical;

extern uint8_t solver_piece_letter[20];
extern uint8_t threeOne[20]; 
extern uint8_t twoOne[20];

extern bool use_seed;
extern int seed;


void initCols(); 
// int* unrank_fun(uint64_t rank, int base, int arraySize);
// uint64_t rank_fun(int* unranked, int base, int arraySize);
// uint64_t pow_u64(unsigned int base, unsigned int exponent);
std::ostream &operator<<(std::ostream &os, uint128_t const &m);


template<typename T>
int* unrank_fun(T rank, int base, int arraySize)
{
    int* unranked = new int[arraySize];
    for(int i = arraySize-1; i >= 0; i--)
    {
        unranked[i] = rank % base;
        rank = rank / base;
    }
    return unranked;
};

template <typename T>
T rank_fun(int* unrank, int base, int arraySize)
{
    T r = 0;
    for(int i = 0; i < arraySize; i++)
    {
        r = r * base + unrank[i];
    }
    return r;
}


template <typename T>
T power(T base, unsigned int exp)
{
    if(exp == 0) return 1;
    
    T res = 1;
    for(;;)
    {
        if(exp & 1)
            res *= base;

        exp >>= 1;

        if(!exp)
            break;
        
        base *= base;
    }
    return res;
}


#endif