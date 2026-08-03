#ifndef PUZZLE_H_
#define PUZZLE_H_
#include "Common.h"
#include "State.h"
#include "TranspositionTable.h"
	
class Puzzle {
	
	std::string _name;
	State _initial;
	std::string reconstruct(int endPos);
	int innerIdaStar(State* s, int depth, uint16_t bound);
	uint16_t threshold;
	uint64_t nodes;
	static int genCalls;
	bool done,timeOut;
	State** path;
	TTInterface *TT=nullptr;
	BFSCache * bfsTT = nullptr;
	bool foundInPreviousTT=false;
	
	
	std::string solution;
	std::chrono::time_point<std::chrono::high_resolution_clock> start;
	float memFactor=(sizeof(State)+sizeof(RowCol)*numPieces+11*sizeof(State*)+16)/1024.0f/1024.0f;
	int64_t totalH=0, countH=0,cutMin=INT_MAX,cutTotal=0,cutCount=0,cutMax=0;
public:
	static std::vector<BFSCache*> TTs;	
	void init();
	Puzzle(std::string name, std::string state);
	Puzzle(std::string name, std::string state, bool order);
	Puzzle(std::string name, const State& state);
	virtual ~Puzzle();
	std::string toString()const;
	void BFS(unsigned int ttSize);
	void setTTForReuse(TTInterface *_TT);

	int idaStar(unsigned int ttSize, unsigned int maxSolLength);

	bool Lookup(const State *s);

	const std::string& getSolution() const {
		return solution;
	}

	bool isTimeOut() const {
		return timeOut;
	}

	std::string statistics();
};

#endif /* PUZZLE_H_ */
