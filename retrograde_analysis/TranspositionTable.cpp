#include "TranspositionTable.h"

TranspositionTableMap::TranspositionTableMap(unsigned int startingSize):mapTT(startingSize){

}
TranspositionTableMap::~TranspositionTableMap(){
	mapTT.clear();
	bestState.clear();
}
void TranspositionTableMap::TTsave(const State * s, int16_t bound)  {
	mapTT[*s]=bound;
}
int16_t TranspositionTableMap::TTLookup(const State * s) const {
	if(mapTT.find(*s)!=mapTT.end()){
		return mapTT.at(*s);
	}else{
		return -1;
	}
}
void TranspositionTableMap::TTmerge(TTInterface* other)
{

	auto otherMap = dynamic_cast< TranspositionTableMap*>(other);
	if(other->isSolvable()) {
		setSolvable();
	}
	mapTT.insert(otherMap->mapTT.cbegin(), otherMap->mapTT.cend());

	
}
std::string TranspositionTableMap::statistics() const
{
	return std::to_string(mapTT.size())+";"+std::to_string(hasSolution);
}


BFSCache::BFSCache(unsigned int startingSize){

}
BFSCache::~BFSCache(){

	for(auto state:setTT)
	{
		delete state;
	}

	setTT.clear();
}
void BFSCache::save(const State *const s){
	setTT.insert(s);
}

bool BFSCache::lookup(const State *const s) const{
	return setTT.find(s) != setTT.end();

}
void BFSCache::merge(BFSCache * other){


	if(other->isSolvable()){
		setSolvable();
	}
	setTT.merge(other->setTT);
	
	/*for(auto st : other->setTT)
	{
		setTT.insert(st);
	}*/
	
	//setTT.insert(other->setTT.begin(), other->setTT.end());
	//other->setTT.clear();
}
std::string BFSCache::statistics() const{
	return std::to_string(setTT.size())+";"+std::to_string(hasSolution);
}

int TTInterface::getBestLength() const
{
	return bestLength;
}
	
std::string TTInterface::getBestState() const
{
	return bestState;
}


void TTInterface::setSolvable()
{
	hasSolution=true;
}

bool TTInterface::isSolvable() const
{
	return hasSolution;
}

void BFSCache::setSolvable()
{
	hasSolution=true;
}

bool BFSCache::isSolvable() const
{
	return hasSolution;
}



