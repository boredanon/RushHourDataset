#ifndef COMMON_H_
#define COMMON_H_

#include <string>

#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>
#include <string.h>
#include <cmath>
#include <sstream>
#include <bitset>
#include <utility>
#include <climits>
#include <chrono>
#include <cinttypes>
#include <random>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#ifdef _WIN32
	#include <windows.h>
	#include <psapi.h>
#else
	#include <sys/time.h>
	#include <sys/resource.h>
#endif
#include <fstream>

//#define GENERATE_RELAXED
#define LOAD_RELAXED
#define BIG



enum ORIENTATION {HOR=0, VERT=1};
enum DIRECTION {UP=0, DOWN=1, LEFT=2, RIGHT=3};
enum PIECE_TYPE {rover=0, two_by_two=1, three_by_one=2, two_by_one=3};
enum HEURISTIC {zero, twoonezero, obstacles0,obstacles1,obstacles2,obstacles3,relaxed1,relaxed2};
extern ORIENTATION ORIENTATIONS[2];
extern DIRECTION DIRECTIONS[4];
extern PIECE_TYPE PIECE_TYPES[4];

extern int timeArg;
extern std::string algorithmArg;
extern HEURISTIC heuristic;
extern bool clusterCache;
extern bool useMapTT;
extern bool connectedComponents;

// const uint64_t outside=3ull|(3ull<<9)|(3ull<<18)|(3ull<<27)|(3ull<<36)|(3ull<<45)|(3ull<<54);
const uint64_t outside=3ull|(3ull<<8)|(3ull<<16)|(3ull<<24)|(3ull<<32)|(3ull<<40);
// const uint64_t outside_exit=3ull|(3ull<<9)|(3ull<<18)/*|(3ull<<27)|(3ull<<36)*/|(3ull<<45)|(3ull<<54);
const uint64_t outside_exit=3ull|(3ull<<8)|(3ull<<16)/*|(3ull<<24)*/|(3ull<<32)|(3ull<<40);
const uint64_t inside=~outside;
const uint64_t inside_exit=~outside_exit;
// const uint64_t goal=(3ull<<27)|(3ull<<36);
const uint64_t goal=(3ull<<24);
// const uint64_t goal_row=(goal<<2)|(goal<<3)|(goal<<4)|(goal<<5)|(goal<<6)|(goal<<7);
// const uint64_t goal_row_positions[8]={//places to the right of this column on the goal row.
// 	(goal)|(goal<<1)|(goal<<2)|(goal<<3)|(goal<<4)|(goal<<5)|(goal<<6)|(goal<<7),
// 	(goal)|(goal<<1)|(goal<<2)|(goal<<3)|(goal<<4)|(goal<<5)|(goal<<6),
// 	(goal)|(goal<<1)|(goal<<2)|(goal<<3)|(goal<<4)|(goal<<5),
// 	(goal)|(goal<<1)|(goal<<2)|(goal<<3)|(goal<<4),
// 	(goal)|(goal<<1)|(goal<<2)|(goal<<3),
// 	(goal)|(goal<<1)|(goal<<2),
// 	(goal)|(goal<<1),
// 	(goal)};
const uint64_t goal_row=(goal<<2)|(goal<<3)|(goal<<4)|(goal<<5)|(goal<<6);
const uint64_t goal_row_positions[7]={//places to the right of this column on the goal row.
	(goal)|(goal<<1)|(goal<<2)|(goal<<3)|(goal<<4)|(goal<<5)|(goal<<6),
	(goal)|(goal<<1)|(goal<<2)|(goal<<3)|(goal<<4)|(goal<<5),
	(goal)|(goal<<1)|(goal<<2)|(goal<<3)|(goal<<4),
	(goal)|(goal<<1)|(goal<<2)|(goal<<3),
	(goal)|(goal<<1)|(goal<<2),
	(goal)|(goal<<1),
	(goal)
};


const uint64_t pre_exit=goal<<2;
const uint64_t pre_pre_exit[3]={pre_exit|(pre_exit<<9),pre_exit|(pre_exit<<1),pre_exit|(pre_exit>>9)};
const uint64_t pre_pre_pre_exit[7]={
		pre_pre_exit[0]|(pre_pre_exit[0]<<9),
		pre_pre_exit[0]|(pre_pre_exit[0]<<1),
		pre_pre_exit[1]|(pre_pre_exit[1]<<9),
		pre_pre_exit[1]|(pre_pre_exit[1]<<1),
		pre_pre_exit[1]|(pre_pre_exit[1]>>9),
		pre_pre_exit[2]|(pre_pre_exit[2]<<1),
		pre_pre_exit[2]|(pre_pre_exit[2]>>9)};
const uint64_t pre_pre_pre_pre_exit[14]={
		pre_pre_pre_exit[0]|(pre_pre_pre_exit[0]<<1),
		pre_pre_pre_exit[1]|(pre_pre_pre_exit[1]<<9),
		pre_pre_pre_exit[1]|(pre_pre_pre_exit[1]<<1),
		pre_pre_pre_exit[2]|(pre_pre_pre_exit[2]<<9),
		pre_pre_pre_exit[2]|(pre_pre_pre_exit[2]<<1),
		//pre_pre_pre_exit[2]|(pre_pre_pre_exit[2]>>1),
		pre_pre_pre_exit[3]|(pre_pre_pre_exit[3]<<9),
		pre_pre_pre_exit[3]|(pre_pre_pre_exit[3]<<1),
		pre_pre_pre_exit[3]|(pre_pre_pre_exit[3]>>9),
		pre_pre_pre_exit[4]|(pre_pre_pre_exit[4]<<1),
		pre_pre_pre_exit[4]|(pre_pre_pre_exit[4]>>9),
		//pre_pre_pre_exit[4]|(pre_pre_pre_exit[4]>>1),
		pre_pre_pre_exit[5]|(pre_pre_pre_exit[5]<<1),
		pre_pre_pre_exit[5]|(pre_pre_pre_exit[5]>>9),
		pre_pre_pre_exit[6]|(pre_pre_pre_exit[6]<<1),
		pre_pre_pre_exit[6]|(pre_pre_pre_exit[6]>>9)};
const uint64_t illegal=~0ull;
const uint8_t illegal_pos=~0;

//masks to extract each part
const uint16_t LETTER=31<<10,ROW=7<<7,COL=15<<3,DIRE=3<<2,TYPE=3;
const uint16_t SHORT_PIECE=(1<<10)-1;

extern PIECE_TYPE piece_type[256];
extern uint8_t piece_letter[20];//index to char
extern uint8_t letter_piece[256];//char to index

//[RowCol][ORIENTATION][TYPE]
extern uint64_t piece_board[128][2][4];

//[type][start][end]
extern uint64_t piece_board_paths[4][128][128];

//[type][orientation][piece]
extern bool exit_obstacles[4][2][128];
extern bool exit_obstacles2[3][4][2][128];
extern bool exit_obstacles3[7][4][2][128];
extern bool exit_obstacles4[16][4][2][128];
extern bool rover_on_exit[4][128];
extern bool init_done;

//[roverRow][roverCol][obsRow][obsCol][type][orientation][obsRow][obsCol][type][orientation]
#ifdef BIG
extern uint8_t relaxationTwo[6][6][7][7][4][2][7][7][4][2];
#endif
extern uint8_t relaxationOne[6][6][7][7][4][2];

extern std::string Problems, times, memories, nodesSearched, initialH, actualSolution;

union RowCol{
	struct _split{
		uint8_t row:3,col:4;
	} split;
	uint8_t packed;
	RowCol():packed(0){

	}
};

//[direction][distance][RowCol][type][orientation]
extern RowCol piece_neighbours[4][7][128][4][2];

extern uint8_t type_sizeX[4][2];
extern uint8_t type_sizeY[4][2];

//filled by State constructor
extern uint8_t index_letter[19];
extern uint8_t letter_index[256];
extern uint8_t numPieces;
extern PIECE_TYPE index_type[19];
extern ORIENTATION index_orientation[19];

//rowcol orientation type 1-2
extern uint64_t zobrist[256][2][4][2];

//board is 7 rows and 9 columns, 2 rightmost columns are outside
uint64_t get_board(RowCol rowCol,ORIENTATION orientation,PIECE_TYPE type);


RowCol move(RowCol rowCol,PIECE_TYPE type,ORIENTATION orientation, DIRECTION direction, uint8_t distance);

void printBoard(uint64_t board);

uint16_t countSetBits(uint64_t i);

struct _dummy_static_initializer{
	_dummy_static_initializer();
};
#endif /* COMMON_H_ */
