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
	for(auto& pair : other->adj) {
		adj[pair.first].insert(pair.second.begin(), pair.second.end());
	}
}

double BFSCache::getLocalCCSum() const
{
	double totalLocalCC = 0.0;

	for(const auto& [s, neighbors] : adj)
	{
		if(s == nullptr) continue;
		int k = neighbors.size();
		if(k < 2){
			continue;
		}

		int actualEdges = 0;
		std::vector<const State*> neighborVec(neighbors.begin(), neighbors.end());
		for(size_t i = 0; i < neighborVec.size(); ++i){
			auto u = neighborVec[i];
			auto uIt = adj.find(u);
			if(uIt == adj.end()) continue;
			const auto& uNeighbors = uIt->second;
			for(size_t j = i + 1; j < neighborVec.size(); ++j){
				auto v = neighborVec[j];

				if(uNeighbors.count(v) > 0){
					actualEdges++;
				}
			}
		}
		double maxPossibleEdges = (static_cast<double>(k) * (k - 1)) / 2.0;
		double localCC = static_cast<double>(actualEdges) / maxPossibleEdges;
		totalLocalCC += localCC;
	}
	return totalLocalCC;
}

int BFSCache::getDiameter() const {
    if (setTT.size() <= 1) return 0;

    int maxDiameter = 0;

    for (const State* startState : setTT) {
        std::unordered_map<const State*, int, std::hash<State*>, std::equal_to<State*>> distances;
        std::queue<const State*> q;

        q.push(startState);
        distances[startState] = 0;

        int localMaxDistance = 0;

        while (!q.empty()) {
            const State* current = q.front();
            q.pop();

            int currentDist = distances[current];
            localMaxDistance = std::max(localMaxDistance, currentDist);

            auto adjIt = adj.find(current);
            if (adjIt == adj.end()) continue;

            for (const State* neighbor : adjIt->second) {
                if (distances.find(neighbor) == distances.end()) {
                    distances[neighbor] = currentDist + 1;
                    q.push(neighbor);
                }
            }
        }
        maxDiameter = std::max(maxDiameter, localMaxDistance);
    }
    return maxDiameter;
}

int BFSCache::getRadius() const {
	std::vector<const State*> nodes;
	for(const auto& [s, neighbors] : adj){
		if(s != nullptr){
			nodes.push_back(s);
		}
	}

	if(nodes.empty()) return 0;
	int minEccentricity = std::numeric_limits<int>::max();

	for(const auto& start : nodes){
		std::unordered_map<const State*, int> distances;
		std::queue<const State*> q;

		distances[start] = 0;
		q.push(start);

		int localMaxDistance = 0;
		while(!q.empty())
		{
			auto current = q.front();
			q.pop();

			int currentDist = distances[current];
			localMaxDistance = std::max(localMaxDistance, currentDist);

			auto it = adj.find(current);
			if(it != adj.end()){
				for(const auto& neighbor : it->second){
					if(neighbor == nullptr) continue;

					if(distances.find(neighbor) == distances.end()){
						distances[neighbor] = currentDist + 1;
						q.push(neighbor);
					}
				}
			}
		}

		int eccentricity = localMaxDistance;
		minEccentricity = std::min(minEccentricity, eccentricity);
	}
	return minEccentricity;
	
}

int BFSCache::getMaxSolutionLength() const
{
    if (!isSolvable()) return -1;

    std::unordered_map<const State*, unsigned short> marked;
    std::unordered_set<const State*> unmarked;

    for (const auto* state : setTT)
    {
        if (state == nullptr) continue;
        if (state->isOneMoveFromGoal()) {
            marked.insert({state, 1});
        }
        else {
            unmarked.insert(state);
        }
    }

    // Edge case: Goal was reached in 0 moves (start state WAS the goal)
    // or no states 1 move away exist in this cache.
    if (marked.empty()) {
        return 0; // Return 0 if start state was already goal, or -1 if unreachable
    }

    int currentDistance = 1;
    bool newNodesAdded = true;

    while (newNodesAdded && !unmarked.empty())
    {
        newNodesAdded = false;
        auto it = unmarked.begin();

        while (it != unmarked.end())
        {
            bool found = false;
            auto adjIt = adj.find(*it);
            
            if (adjIt != adj.end())
            {
                for (const auto* neighbor : adjIt->second)
                {
                    if (neighbor == nullptr) continue;

                    auto search = marked.find(neighbor);
                    if (search != marked.end() && search->second == currentDistance)
                    {
                        marked.insert({*it, static_cast<unsigned short>(currentDistance + 1)});
                        found = true;
                        newNodesAdded = true;
                        break; // Step found; move to next unmarked node
                    }
                }
            }

            if (!found) {
                ++it;
            }
            else {
                it = unmarked.erase(it); // Remove marked state from unmarked set
            }
        }

        if (newNodesAdded) {
            currentDistance++;
        }
    }

    return currentDistance;
}

double BFSCache::getBranchingFactor() const
{
	double totalDegree = 0.0;
	int nodeCount = 0.0;

	for(const auto& [s, neighbors] : adj){
		if(s == nullptr) continue;
		nodeCount++;

		int validNeighbors = 0;
		for(const auto* neighbor: neighbors){
			if(neighbor != nullptr){
				validNeighbors++;
			}
		}

		totalDegree += validNeighbors;
	}

	if(nodeCount == 0) return 0.0;

	return totalDegree / static_cast<double>(nodeCount);
}

double BFSCache::getFiedlerValue() const
{
	std::vector<const State*> nodes;
    for (const auto& [s, neighbors] : adj) {
        if (s != nullptr) {
            nodes.push_back(s);
        }
    }

	int n = nodes.size();
    if (n < 2) return 0.0;

	std::unordered_map<const State*, int> stateToIndex;
	stateToIndex.reserve(n);
    for (int i = 0; i < n; ++i) {
        stateToIndex[nodes[i]] = i;
    }

	Eigen::MatrixXd L = Eigen::MatrixXd::Zero(n, n);

    for (int i = 0; i < n; ++i) {
        const auto* u = nodes[i];
        auto it = adj.find(u);
        if (it == adj.end()) continue;

        int degree = 0;
        for (const auto* v : it->second) {
            if (v == nullptr) continue;
            
            auto vIt = stateToIndex.find(v);
			if(vIt != stateToIndex.end())
			{
				int j = vIt->second;
				L(i, j) -= 1.0;
				degree++;
			}
        }
        L(i, i) = degree; // Degree contribution (D)
    }

	Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> solver(L);
    if (solver.info() != Eigen::Success) {
        return 0.0; // Solver failed
    }

	return solver.eigenvalues()(1);
}


std::string BFSCache::statistics() const{
	return std::to_string(size())+","+std::to_string(hasSolution)+","+std::to_string(getLocalCCSum()/size())+","
	+std::to_string(getDiameter())+",";
}

int BFSCache::getExits() const {
	int exits = 0;
	for(auto& state : setTT)
	{
		if(state->isOneMoveFromGoal())
		{
			exits++;
		}
	}
	return exits;
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



