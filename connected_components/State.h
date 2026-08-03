#ifndef STATE_H_
#define STATE_H_
#include "Common.h"
#include <memory>
class State {
	const RowCol *const pieces;
	const uint64_t hash;
	uint64_t calculateZobristHash(int n) const;
	uint16_t h;

	uint16_t calculateH() const;
	const RowCol* getPieces(const std::string s) const;
	const RowCol* getPieces2(const std::string s) const;
	static RowCol* copyPieces(const RowCol* prevPieces);
	uint16_t getTwoOne(const RowCol& rover) const;
public:
	
	uint64_t getHash() const;
	uint16_t getH() const;
	void setH(uint16_t h);
	std::vector<State*> genSuccesors() const;
	bool isGoal() const;
	bool isOneMoveFromGoal() const;
	State(const std::string s);
	State(const std::string s, bool ordered);
	State(const State &s);
	State(const RowCol *pieces);
	State(const RowCol *pieces, uint64_t hash);
	std::string toString()const;
	const std::string getMove(const State *const next)const;
	State* applyMove(const std::string move)const;
	virtual ~State();
	friend class std::equal_to<State*>;
	friend class std::equal_to<State>;
	const RowCol* getPiecesCopy() const;
};
class StatePointerComparator
{
public:
	StatePointerComparator(){}
	bool operator ()(const State* lhs, const State* rhs) const;
};
class HighFLowGComparator
{
public:
	HighFLowGComparator(){}
	bool operator ()(const State* lhs, const State* rhs) const;
};

class ReverseStatePointerComparator
{
public:
	ReverseStatePointerComparator(){}
	bool operator ()(const State* lhs, const State* rhs) const;
};
namespace std {
template <>struct hash<State*>{
	size_t operator()(const State* s) const{return s->getHash();}
};


template <>struct equal_to<State*>{
	bool operator() (const State* x, const State* y) const;
};


template <>struct hash<State>{
	size_t operator()(const State& s) const{return  hash<State*>{}(&s);}
};
template <>struct equal_to<State>{
	bool operator() (const State& x, const State& y) const;
};

template <>struct hash<std::shared_ptr<State>>{
bool operator()(const std::shared_ptr<State> x) const {
        return hash<State*>()(x.get());
	};
};

template <>struct equal_to<std::shared_ptr<State>>{
bool operator()(const std::shared_ptr<State> x, const std::shared_ptr<State> y) const {
        return equal_to<State*> ()((State*)x.get(), (State*)y.get());
	};
};

}

#endif /* STATE_H_ */
