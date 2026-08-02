#ifndef TRANSPOSITIONTABLE_H_
#define TRANSPOSITIONTABLE_H_
#include "Common.h"
#include "State.h"
#include<unordered_map>
#include<unordered_set>
#include<boost/container_hash/hash.hpp>


class Entry{
private:
	const State* state;
	int16_t bound;
public:
	Entry(const State* state,int16_t bound):
				state(state),
				bound(bound){}
	Entry():
		state(nullptr),
		bound(SHRT_MAX){}

	const State* getState() const {
		return state;
	}
	int16_t getBound() const {
			return bound;
		}

};

class TTInterface
{
	protected:
		std::string bestState;
		int bestLength = -1;
		bool hasSolution=false;
	public:
		
		int getBestLength() const;
		std::string getBestState() const;
		void setSolvable();
		bool isSolvable() const;
		virtual ~TTInterface(){};
		virtual void TTsave(const State * s, int16_t bound) = 0;
		virtual int16_t TTLookup(const State * s) const = 0;
		virtual void TTmerge( TTInterface * other) = 0;
		virtual std::string statistics() const = 0;
};

class TranspositionTableMap : public TTInterface{ 
	std::unordered_map<State,int16_t> mapTT;
	public:
	TranspositionTableMap(unsigned int startingSize);
	virtual ~TranspositionTableMap();
	virtual void TTsave(const State * s, int16_t bound);
	virtual int16_t TTLookup(const State * s) const;
	virtual void TTmerge(TTInterface * other);
	virtual std::string statistics() const;
	
};

class BFSCache{
	
	bool hasSolution=false;
	public:
	std::unordered_set<const State*, std::hash<State*>, std::equal_to<State*>> setTT;
	int size() const { return setTT.size(); };
	const State* getFirst() const { return *setTT.begin();};
	void setSolvable();
	bool isSolvable() const;
	BFSCache(unsigned int startingSize);
	~BFSCache();
	void save(const State *const s);
	bool lookup(const State *const s) const;
	void merge(BFSCache * other);
	std::string statistics() const;
	

};

#endif /* TRANSPOSITIONTABLE_H_ */
