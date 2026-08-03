#include "Puzzle.h"
#include <algorithm>
#include <cassert>

std::string Problems, times, memories, nodesSearched, initialH, actualSolution;
std::vector<BFSCache*> Puzzle::TTs;
//BFSCache* bfsTT = nullptr;
int Puzzle::genCalls = 0;
//BFSCache * bfsTT=nullptr;	
//bool foundInPreviousTT=false;

Puzzle::Puzzle(std::string name, std::string state):_name(name), _initial(State(state)) {


}

Puzzle::Puzzle(std::string name, std::string state, bool order):_name(name), _initial(State(state, order)) {


}

Puzzle::~Puzzle() {
	// TODO Auto-generated destructor stub
}

std::string Puzzle::toString() const {
	std::string s("P ");
	s.append(_name);
	s.append("\n");
	std::string state= _initial.toString();
	s.append(state);
	return s;
}


int Puzzle::innerIdaStar(State* s, int depth, uint16_t bound) {
	nodes++;
	if(nodes%5000==0){
		if(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() -start).count()>timeArg*1000){
			timeOut=true;
			return -1;
		}
	}

	if (s->isGoal()){// goal found
		done=true;
		path[depth]=new State(*s);
		TT->TTsave(s, 0);
		return 0;
	}

	

	const std::vector<State*> &children=s->genSuccesors();


	for (auto it=children.begin(); it!=children.end();it++) {
		auto newH=TT->TTLookup(*it);
		genCalls++;
		if (newH>=0){
		// 		auto newH=entry.getBound();
			if(newH>(*it)->getH()){
				if(depth<cutMin){
					cutMin=depth;
				}
				if(depth>cutMax){
					cutMax=depth;
				}
				cutTotal+=depth;
				cutCount++;
			}
			(*it)->setH(newH);

		}
	}

	uint16_t new_bound=SHRT_MAX;

	for (auto it=children.begin(); it!=children.end();it++) {
		if(timeOut){
			for(auto toDel=it;toDel!=children.end();toDel++){
				delete (*toDel);
			}
			return -1;
		}

		uint16_t t;
		totalH+=(*it)->getH();
		countH++;
		if ((*it)->getH() <= bound-1) {
			t = 1 + innerIdaStar(*it, depth+1, bound - 1);
		} else {
			t = 1 + (*it)->getH();
		}
		delete (*it);
		if (done) {
			path[depth]=new State(*s);
			for(auto toDel=it+1;toDel!=children.end();toDel++){
				delete (*toDel);
			}
			return t;
		}

		if (t < new_bound) {
			new_bound = t ; // better lower bound
		}
	}


	solution="Can't find answer";
	TT->TTsave(s,new_bound);
	return new_bound;
}

std::string Puzzle::reconstruct(int endPos) {
	std::stringstream ss;
	for(int i=0;i<endPos;i++){
		ss<<" "<<path[i]->getMove(path[i+1]);
	}
	return ss.str();
}

Puzzle::Puzzle(std::string name,const State& state):_name(name), _initial(state) {
}

void Puzzle::init() {
	start=std::chrono::high_resolution_clock::now();
	timeOut=false;
	nodes=0;
	threshold=0;
}

void Puzzle::setTTForReuse(TTInterface*_TT){
	TT= _TT;
}

int Puzzle::idaStar(unsigned int ttSize, unsigned int maxSolLength) {
	if (TT==nullptr){
			TT=new TranspositionTableMap(ttSize);
	}
	

	path=new State*[maxSolLength];
	threshold = _initial.getH();

	//Let's see if we can improve threshold it from previous searches
	
	auto newH=TT->TTLookup(&_initial);
	genCalls++;
	if (newH>=0){ 
		// auto newH=entry.getBound();
		if(newH>_initial.getH()){
			threshold = newH;
		}
	}	
		
	
	done = false;
	int moves;

	while (!done) {
		moves = innerIdaStar(&_initial,0, threshold);
		if (!done){
			threshold = moves;
			if(threshold>maxSolLength){
				moves=-1;
				break;
			}
		}
		if(moves<0){
			break;
		}
	}
	if(moves>=0){
		solution=reconstruct(moves);
		for(int i=0;i<=moves;i++){
			delete path[i];
		}
	}
	Problems.append("'").append(_name).append("',");
	delete[] path;
	return moves;
}

void Puzzle::BFS(unsigned int ttSize)
{
	// assert(useBFS);
	foundInPreviousTT = false;
	//bfsTT = nullptr;
	if(Lookup(&_initial)){
		return;
	}else
	{
		bfsTT = new BFSCache(ttSize);
	}

	//innerBFS(&_initial);
	std::unordered_map<const State*, const State*, std::hash<State*>, std::equal_to<State*>> canonical_map;
	std::unordered_set<State*, std::hash<State*>, std::equal_to<State*>> set;
	std::deque<State*> queue;

	State* copy = new State(_initial);
	set.insert(copy); 
	queue.push_back(copy);
	canonical_map[copy] = copy;
	
	while(!queue.empty())
	{
		auto s = queue.front();
		queue.pop_front();
		// set.erase(s);

		if(s->isGoal()){
			bfsTT->setSolvable();
			delete s;
			continue;
		}

		if(Lookup(s)){
			// Do not delete s to avoid dangling pointers in adj
			continue;
		}
		bfsTT->save(s);
		const std::vector<State*> &children=(s)->genSuccesors();
		for (auto it=children.begin(); it!=children.end();it++) {
			if(*it == nullptr){
				continue;
			}
			if((*it)->isGoal()){
				bfsTT->setSolvable();
				delete *it;
				continue;
			}
			genCalls++;

			const State* canonical_child = nullptr;
			auto mapIt = canonical_map.find(*it);
			if(mapIt != canonical_map.end()){
				canonical_child = mapIt->second;
			}

			if(!Lookup(*it) && (set.count(*it) <= 0))
			{
				queue.push_back(*it); set.insert(*it);
				canonical_map[*it] = *it;
				canonical_child = *it;
			}
			else
			{
				if(canonical_child == nullptr){
					canonical_child = bfsTT->getCanonical(*it);
					if (canonical_child == nullptr) {
						delete *it;
						continue;
					}
				}
				delete *it;
			}

			if(s!=nullptr && canonical_child != nullptr){
				bfsTT->adj[s].insert(canonical_child);
				bfsTT->adj[canonical_child].insert(s);
			}
		}
		//delete s;
	}


	if(find(TTs.begin(), TTs.end(), bfsTT) == TTs.end())
	{
		TTs.push_back(bfsTT);
	}
}



bool Puzzle::Lookup(const State * s)
{
	if(bfsTT!=nullptr && bfsTT->lookup(s)){
		return true;
	}else if(bfsTT == nullptr)
	{
		for(auto it = TTs.begin(); it != TTs.end(); it++)
		{
			if((*it)->lookup(s))
			{
				bfsTT = *it;
				return true;
			}
		}
	}

	if(!foundInPreviousTT)
	{
		for(auto it = TTs.begin(); it != TTs.end(); it++)
		{
			if((*it)->lookup(s))
			{
				BFSCache* oldTT = *it;
				oldTT->merge(bfsTT);
				auto ttIt = std::find(std::begin(TTs), std::end(TTs), bfsTT);
				if(ttIt != std::end(TTs)) {
					TTs.erase(ttIt);
				}
				delete bfsTT;
				bfsTT = oldTT;
				foundInPreviousTT = true;
				return true;
			}
			
		}
	}
	else
	{
		for(auto it = TTs.begin(); it != TTs.end(); it++)
		{
			if(*it == bfsTT) continue; // self
			if((*it)->lookup(s))
			{
				bfsTT->merge(*it);
				TTs.erase(it);
				return true;
			}
		}
			
	}

	return false;
}


std::string Puzzle::statistics() {
	// nodes searched,
	//execution time,
	//average h-value for all nodes where an evaluation occurred in the tree,
	//and min/average/max depth of nodes where a cutoff occurred
	std::stringstream ss;
	ss<<"Nodes: "<<nodes<<", average H: "<<totalH/(float)countH<<", cut-off depth (min/average/max): "<<
			cutMin<<"/"<<cutTotal/(float)cutCount<<"/"<<cutMax;

	return ss.str();
}
