#include "Utils.h"

int boardSizes[2] = {6/*width*/, 6/*height*/};
int minRows[6] = {0, 1, 5 , 8, 11, 16};
int maxRows[6] = {1, 5, 8, 11, 16, 22};

//todo: revisar si se llena de ceros a la derecha o izquierda
std::bitset<36> rowBits[]  {
    std::bitset<36>("000000"),//empty
    std::bitset<36>("111000"),//3x1
    std::bitset<36>("011100"),
    std::bitset<36>("001110"), 
    std::bitset<36>("000111"), 
    std::bitset<36>("111110"), //3x1 2x1
    std::bitset<36>("111011"),
    std::bitset<36>("011111"),
    std::bitset<36>("111110"), //2x1 3x1
    std::bitset<36>("110111"),
    std::bitset<36>("011111"),
    std::bitset<36>("110000"),//2x1
    std::bitset<36>("011000"),
    std::bitset<36>("001100"),
    std::bitset<36>("000110"),
    std::bitset<36>("000011"),
    std::bitset<36>("111100"), // 2x1 2x1
    std::bitset<36>("110110"),
    std::bitset<36>("110011"), 
    std::bitset<36>("011110"),
    std::bitset<36>("011011"), 
    std::bitset<36>("001111")/*,
    std::bitset<36>("111111") // 2x1 2x1 2x1*/
};

std::bitset<36> rowBitsOneTwo[]  {
    std::bitset<36>("000000"),//empty
    std::bitset<36>("000000"),//3x1
    std::bitset<36>("000000"),
    std::bitset<36>("000000"), 
    std::bitset<36>("000000"), 
    std::bitset<36>("000110"), //3x1 2x1
    std::bitset<36>("000011"),
    std::bitset<36>("000011"),
    std::bitset<36>("110000"), //2x1 3x1
    std::bitset<36>("110000"),
    std::bitset<36>("011000"),
    std::bitset<36>("110000"),//2x1
    std::bitset<36>("011000"),
    std::bitset<36>("001100"),
    std::bitset<36>("000110"),
    std::bitset<36>("000011"),
    std::bitset<36>("111100"), // 2x1 2x1
    std::bitset<36>("110110"),
    std::bitset<36>("110011"),
    std::bitset<36>("011110"), 
    std::bitset<36>("011011"), 
    std::bitset<36>("001111")/*,
    std::bitset<36>("111111") // 2x1 2x1 2x1*/
};

std::bitset<36> rowBitsOneThree[]  {
    std::bitset<36>("000000"),//empty
    std::bitset<36>("111000"),//3x1
    std::bitset<36>("011100"),
    std::bitset<36>("001110"), 
    std::bitset<36>("000111"), 
    std::bitset<36>("111000"), //3x1 2x1
    std::bitset<36>("111000"),
    std::bitset<36>("011100"),
    std::bitset<36>("001110"), //2x1 3x1
    std::bitset<36>("000111"),
    std::bitset<36>("000111"),
    std::bitset<36>("000000"),//2x1
    std::bitset<36>("000000"),
    std::bitset<36>("000000"),
    std::bitset<36>("000000"),
    std::bitset<36>("000000"),
    std::bitset<36>("000000"), // 2x1 2x1
    std::bitset<36>("000000"),
    std::bitset<36>("000000"),
    std::bitset<36>("000000"), 
    std::bitset<36>("000000"), 
    std::bitset<36>("000000")/*,
    std::bitset<36>("000000") // 2x1 2x1 2x1*/
};

std::bitset<36> colBits[22];
std::bitset<36> colBitsOneTwo[22];
std::bitset<36> colBitsOneThree[22];

std::vector<char> piecesThreebyOneVertical;
std::vector<char> piecesThreebyOneHorizontal;
std::vector<char> piecesTwobyOneVertical;
std::vector<char> piecesTwobyOneHorizontal;
uint8_t threeOne[20] = {'o', 'p', 'q', 'r', 's'}; 
uint8_t twoOne[20] = {'a', 'f', 'd', 'g', 'j', 'b', 'h', 'i', 'k', 'c', 'e'};

uint8_t solver_piece_letter[20]={'_','u','v','x','o', 'q', 's','p', 'r','a', 'f','d',
		'g', 'j','b', 'h', 'i', 'k','c', 'e'};

bool use_seed;
int seed;

void initCols(){
    for(int col=0;col<22;col++){

        for(int i=0;i<boardSizes[0];i++){
            colBits[col][i * boardSizes[0]]=rowBits[col][i];
            colBitsOneTwo[col][i * boardSizes[0]]=rowBitsOneTwo[col][i];
            colBitsOneThree[col][i * boardSizes[0]]=rowBitsOneThree[col][i];
        }
    }
};

std::ostream &operator<<(std::ostream &os, uint128_t const &m) { 
    uint64_t lo = m;
    uint64_t hi = m >> 64;
    
    return os << std::hex << hi << std::hex << lo << std::dec;
};
