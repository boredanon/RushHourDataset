#ifndef PUZZLESTATE_H_
#define PUZZLESTATE_H_
#include "Utils.h"
#include "State.h"

class PuzzleState{
    std::bitset<36> piecesOneTwo;
    std::bitset<36> piecesOneThree;
    std::bitset<36> vert;

    public:
    void print() const;
    bool addRow(int rowType, int rowPos);
    bool addCol(int colType, int colPos);
    static PuzzleState* makeState(int rows[7],int cols[7]);
    std::string ToStringSolvable() const;
};
#endif