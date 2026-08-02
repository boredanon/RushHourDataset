#include <iostream>
#include <fstream>
#include <unordered_set>
#include <boost/program_options.hpp>
#include <cmath>
#include <limits>
#include <unordered_map>
#include <algorithm>
#include <numeric>
#include <random>

#include "Puzzle.h"
#include "PuzzleState.h"
#include "Utils.h"
#include "Common.h"

namespace po = boost::program_options;

int timeArg=86400,memoryArg=1000000,ttSizeArg=10000000,maxSolLength=1000;
std::string algorithmArg="Astar",heuristicArg="obstacles0";
HEURISTIC heuristic=obstacles0;
int printedBoardLimit;
float discountFactor = 1.0f;


std::unordered_map<State*, float> visited;
std::unordered_set<State*> path; 
std::string entropyAlgorithm; 


float calculateStateUniformEntropy(const State* s)
{
    if(s->isGoal() || s->isOneMoveFromGoal())
    {
        return 0.0f;
    }
    auto children = s->genSuccesors();
    auto size = children.size();
    for(const auto& c : children) { delete c; }

    if(size == 0) {
        return std::numeric_limits<float>::infinity(); // Infinite entropy for dead-end states
    }


    auto prob = 1.0f / size;
    return size * prob * log2(1/prob);
}

float calculateStateRelativeUniformEntropy(const State *s, std::vector<float>& childEntropies)
{
    float min_z = std::numeric_limits<float>::infinity();
    for(const auto & z : childEntropies) {
        if(z < min_z) {
            min_z = z;
        }
    }

    if(min_z == std::numeric_limits<float>::infinity()){
        return std::numeric_limits<float>::infinity();
    }


    float shifted_denominator = 0.0f;
    for(const auto & z : childEntropies) {
        if(z != std::numeric_limits<float>::infinity()){
            shifted_denominator += exp(-(z - min_z));
        }
    }

    float kl_divergence = 0.0f;
    double logQ = log2(1.0 / childEntropies.size());
    
    for(const auto & z : childEntropies) {  
        if(z != std::numeric_limits<float>::infinity()){
            float p_i = exp(-(z - min_z)) / shifted_denominator;
            kl_divergence += p_i * (log2(p_i) - logQ);
        }
    }
    return kl_divergence;
}

float calculateStateImmediateEntropy(const State* s, std::vector<float>& childEntropies, const std::string& algorithm)
{
    if(algorithm == "reMUSE"){
        return calculateStateRelativeUniformEntropy(s, childEntropies);
    }
    else if(algorithm == "MUSE")
    {
       return calculateStateUniformEntropy(s);
    }
    return 0.0f;
}

bool checkCardinality(int* b)
{
    int three_by_ones = 0;
    int two_by_ones = 0; 
    for(int i = 0; i < 12; i++)
    {
        if(b[i] == 1){
            three_by_ones+=1;
        }
        else if( (b[i] == 2 || b[i] == 3)){
            three_by_ones+=1;
            two_by_ones+=1;
        }
        else if(b[i] == 4){
            two_by_ones+=1;
        }
        else if(b[i] == 5){
            two_by_ones+=2;
        }
    }
    return three_by_ones <= 4 && two_by_ones <= 12;
}

std::unordered_set<PuzzleState*> getPuzzleStates(int* c)
{
    int dims = 12;
    std::unordered_set<PuzzleState*> boards;
    std::vector<int> board(dims);
    for(int d = 0; d < dims; d++)
    {
        board[d] = maxRows[c[d]] - 1;
    }


    while(true){
        PuzzleState* state = PuzzleState::makeState(board.data()+6, board.data());
        if(state != nullptr)
        {
            boards.insert(state);
        }

        int pos = dims-1;
        while(pos >= 0)
        {
            board[pos]--;
            if(board[pos] >= minRows[c[pos]]){
                break;
            }else
            {
                board[pos] = maxRows[c[pos]] - 1;
                pos--;
            }
        }
        if (pos < 0) break;

    }
    return boards;
}

float retrogradeAnalysisEntropy(uint64_t cluster)
{
    auto start = std::chrono::high_resolution_clock::now();

    // Add the red car line.
    int* unranked = new int[12];
    auto temp = unrank_fun<uint64_t>(cluster, 6, 11);
    std::copy(temp, temp+8, unranked);
    unranked[8] = 4;
    std::copy(temp+8, temp+11, unranked+9);
    uint64_t true_rank = rank_fun<uint64_t>(unranked, 6, 12);
    delete [] temp;

    // Evaluate if cluster is feasible.
    if(!checkCardinality(unranked))
    {
        delete [] unranked;
        return std::numeric_limits<float>::infinity();
    }

    // Prepare solver variables.
    piecesThreebyOneHorizontal.clear();
    piecesTwobyOneHorizontal.clear();
    piecesThreebyOneVertical.clear();
    piecesTwobyOneVertical.clear();

    int indexTh=0;
    int indexTw=0;

    for(int v = 0; v < 6; v++) // Verticals
    {
       switch(unranked[v])
       {
           case 1: // a, 3x1
               piecesThreebyOneVertical.push_back(static_cast<char>(threeOne[indexTh++])); 
           break;

           case 2: // b, c, 3x1, 2x1
           case 3:
               piecesThreebyOneVertical.push_back(static_cast<char>(threeOne[indexTh++]));
               piecesTwobyOneVertical.push_back(static_cast<char>(twoOne[indexTw++]));
           break;

           case 4: // d, 2x1
               piecesTwobyOneVertical.push_back(static_cast<char>(twoOne[indexTw++])); 
           break;

           case 5: // e, 2x1, 2x1
               piecesTwobyOneVertical.push_back(static_cast<char>(twoOne[indexTw++]));
               piecesTwobyOneVertical.push_back(static_cast<char>(twoOne[indexTw++]));
           break;
       }
    }

    for(int h = 6; h < 12; h++) // Horizontals
    {
        if(h == 8) continue; // x
        switch(unranked[h])
        {
            case 1: // a, 3x1
                piecesThreebyOneHorizontal.push_back(static_cast<char>(threeOne[indexTh++])); 
            break;

            case 2: // b, c, 3x1, 2x1
            case 3:
                piecesThreebyOneHorizontal.push_back(static_cast<char>(threeOne[indexTh++]));
                piecesTwobyOneHorizontal.push_back(static_cast<char>(twoOne[indexTw++]));
            break;

            case 4: // d, 2x1
                piecesTwobyOneHorizontal.push_back(static_cast<char>(twoOne[indexTw++])); 
            break;

            case 5: // e, 2x1, 2x1
                piecesTwobyOneHorizontal.push_back(static_cast<char>(twoOne[indexTw++])); 
                piecesTwobyOneHorizontal.push_back(static_cast<char>(twoOne[indexTw++])); 
            break;
        }
    }

    auto states = getPuzzleStates(unranked);

    std::unordered_map<State*, float> cumulativeEntropyMap;
    std::unordered_set<State*> open;
    
    bool hasSolution = false;

    for(const auto& state : states)
    {
        State* s = new State(state->ToStringSolvable(), true);
        if(s->isOneMoveFromGoal() || s->isGoal()){
            cumulativeEntropyMap[s] = 0.0f;
            hasSolution = true;
        }
        else{
            cumulativeEntropyMap[s] = std::numeric_limits<float>::infinity();
            open.insert(s);
        }
        delete state;
    }

    float globalMaxMinEntropy = 0.0f;
    if(!hasSolution){
        for(const auto& entry : cumulativeEntropyMap){
            delete entry.first;
        }
        cumulativeEntropyMap.clear();
        delete [] unranked;
        return std::numeric_limits<float>::infinity();
    }
    else
    {
        bool mapsChanged = true;
        while(mapsChanged)
        {
            mapsChanged = false;
            for(auto it = open.begin(); it != open.end(); it++)
            {
                float currentEntropy = cumulativeEntropyMap[*it]; 
                float localEntropy = calculateStateUniformEntropy(*it);

                auto children = (*it)->genSuccesors();
                float minChildEntropy = std::numeric_limits<float>::infinity();


                for(const auto& child : children) 
                {
                    float childPathEntropy = cumulativeEntropyMap[child];
                    if(childPathEntropy != std::numeric_limits<float>::infinity()){
                        float totalEntropy = (localEntropy + childPathEntropy) * discountFactor; // discount factor.
                        if(totalEntropy < minChildEntropy)
                        {
                            minChildEntropy = totalEntropy;
                        }
                    }
                    delete child;
                }
                
                if(minChildEntropy < currentEntropy)
                {
                    cumulativeEntropyMap[*it] = minChildEntropy;
                    mapsChanged = true;
                }
            }
        }
        
        // Calculate global max minimum entropy for return value
        for(const auto& entry : cumulativeEntropyMap) {
            if(entry.second != std::numeric_limits<float>::infinity() && entry.second > globalMaxMinEntropy) {
                globalMaxMinEntropy = entry.second;
            }
        }
    }
    std::vector<std::pair<State*, float>> sortedEntries;
    for(const auto & entry : cumulativeEntropyMap)
    {
        State* s = entry.first;
        float e = entry.second;
        if(e != std::numeric_limits<float>::infinity()){
            sortedEntries.push_back({s, e});
        }
    }

    std::sort(sortedEntries.begin(), sortedEntries.end(), 
    [](const std::pair<State*, float>& a, const std::pair<State*, float>& b) {
            return a.second > b.second; // Ascending: minimum values first
        }
    );

    int boardsPrinted = 0;
    float lastPrintedMetric = -std::numeric_limits<float>::infinity();
    
    for (const auto& entry : sortedEntries) {
        if(boardsPrinted >= printedBoardLimit){
            break;
        }
        if(entry.second != lastPrintedMetric){
            std::cout << "==============" << std::endl;
            std::cout << "Cluster: 0x" << std::hex << true_rank << " (converted to 0x" << cluster << std::dec << ")\n";
            std::cout << "Entropy: " << cumulativeEntropyMap[entry.first] << "\n";
        }
        std::cout << "State: \n" << entry.first->toString() << std::endl;
        boardsPrinted++;
    }

    for(const auto& entry : cumulativeEntropyMap){
        delete entry.first;
    }
    cumulativeEntropyMap.clear();
    open.clear();

    delete [] unranked;
    return globalMaxMinEntropy;
}

float retrogradeAnalysisLength(uint64_t cluster)
{
    auto start = std::chrono::high_resolution_clock::now();

    // Add the red car line.
    int* unranked = new int[12];
    auto temp = unrank_fun<uint64_t>(cluster, 6, 11);
    std::copy(temp, temp+8, unranked);
    unranked[8] = 4;
    std::copy(temp+8, temp+11, unranked+9);
    uint64_t true_rank = rank_fun<uint64_t>(unranked, 6, 12);
    delete [] temp;

    // Evaluate if cluster is feasible.
    if(!checkCardinality(unranked))
    {
        delete [] unranked;
        return std::numeric_limits<float>::infinity();
    }

    // Prepare solver variables.
    piecesThreebyOneHorizontal.clear();
    piecesTwobyOneHorizontal.clear();
    piecesThreebyOneVertical.clear();
    piecesTwobyOneVertical.clear();

    int indexTh=0;
    int indexTw=0;

    for(int v = 0; v < 6; v++) // Verticals
    {
       switch(unranked[v])
       {
           case 1: // a, 3x1
               piecesThreebyOneVertical.push_back(static_cast<char>(threeOne[indexTh++])); 
           break;

           case 2: // b, c, 3x1, 2x1
           case 3:
               piecesThreebyOneVertical.push_back(static_cast<char>(threeOne[indexTh++]));
               piecesTwobyOneVertical.push_back(static_cast<char>(twoOne[indexTw++]));
           break;

           case 4: // d, 2x1
               piecesTwobyOneVertical.push_back(static_cast<char>(twoOne[indexTw++])); 
           break;

           case 5: // e, 2x1, 2x1
               piecesTwobyOneVertical.push_back(static_cast<char>(twoOne[indexTw++]));
               piecesTwobyOneVertical.push_back(static_cast<char>(twoOne[indexTw++]));
           break;
       }
    }

    for(int h = 6; h < 12; h++) // Horizontals
    {
        if(h == 8) continue; // x
        switch(unranked[h])
        {
            case 1: // a, 3x1
                piecesThreebyOneHorizontal.push_back(static_cast<char>(threeOne[indexTh++])); 
            break;

            case 2: // b, c, 3x1, 2x1
            case 3:
                piecesThreebyOneHorizontal.push_back(static_cast<char>(threeOne[indexTh++]));
                piecesTwobyOneHorizontal.push_back(static_cast<char>(twoOne[indexTw++]));
            break;

            case 4: // d, 2x1
                piecesTwobyOneHorizontal.push_back(static_cast<char>(twoOne[indexTw++])); 
            break;

            case 5: // e, 2x1, 2x1
                piecesTwobyOneHorizontal.push_back(static_cast<char>(twoOne[indexTw++])); 
                piecesTwobyOneHorizontal.push_back(static_cast<char>(twoOne[indexTw++])); 
            break;
        }
    }

    auto states = getPuzzleStates(unranked);

    std::unordered_map<State*, float> cumulativeLengthMap;
    std::unordered_set<State*> open;
    
    bool hasSolution = false;

    for(const auto& state : states)
    {
        State* s = new State(state->ToStringSolvable(), true);
        if(s->isOneMoveFromGoal() || s->isGoal()){
            cumulativeLengthMap[s] = 1.0f;
            hasSolution = true;
        }
        else{
            cumulativeLengthMap[s] = std::numeric_limits<float>::infinity();
            open.insert(s);
        }
        delete state;
    }

    float globalMaxMinLength = 0.0f;
    if(!hasSolution){
        for(const auto& entry : cumulativeLengthMap){
            delete entry.first;
        }
        cumulativeLengthMap.clear();
        delete [] unranked;
        return std::numeric_limits<float>::infinity();
    }
    else
    {
        bool mapsChanged = true;
        while(mapsChanged)
        {
            mapsChanged = false;
            for(auto it = open.begin(); it != open.end(); it++)
            {
                float currentLength = cumulativeLengthMap[*it]; 
                float localLength = 1.0f;

                auto children = (*it)->genSuccesors();
                float minChildLength = std::numeric_limits<float>::infinity();


                for(const auto& child : children) 
                {
                    float childPathLength = cumulativeLengthMap[child];
                    if(childPathLength != std::numeric_limits<float>::infinity()){
                        float totalEntropy = localLength + childPathLength;
                        if(totalEntropy < minChildLength)
                        {
                            minChildLength = totalEntropy;
                        }
                    }
                    delete child;
                }
                
                if(minChildLength < currentLength)
                {
                    cumulativeLengthMap[*it] = minChildLength;
                    mapsChanged = true;
                }
            }
        }
        
        // Calculate global max minimum entropy for return value
        for(const auto& entry : cumulativeLengthMap) {
            if(entry.second != std::numeric_limits<float>::infinity() && entry.second > globalMaxMinLength) {
                globalMaxMinLength = entry.second;
            }
        }
    }
    std::vector<std::pair<State*, float>> sortedEntries;
    for(const auto & entry : cumulativeLengthMap)
    {
        State* s = entry.first;
        float e = entry.second;
        if(e != std::numeric_limits<float>::infinity()){
            sortedEntries.push_back({s, e});
        }
    }

    std::sort(sortedEntries.begin(), sortedEntries.end(), 
    [](const std::pair<State*, float>& a, const std::pair<State*, float>& b) {
            return a.second > b.second; // Descending.
        }
    );

    int boardsPrinted = 0;
    float lastPrintedMetric = -std::numeric_limits<float>::infinity();
    
    for (const auto& entry : sortedEntries) {
        if(boardsPrinted >= printedBoardLimit){
            break;
        }
        if(entry.second != lastPrintedMetric){
            std::cout << "==============" << std::endl;
            std::cout << "Cluster: 0x" << std::hex << true_rank << " (converted to 0x" << cluster << std::dec << ")\n";
            std::cout << "Length: " << cumulativeLengthMap[entry.first] << "\n";
        }
        std::cout << "State: \n" << entry.first->toString() << std::endl;
        boardsPrinted++;
    }

    for(const auto& entry : cumulativeLengthMap){
        delete entry.first;
    }
    cumulativeLengthMap.clear();
    open.clear();

    delete [] unranked;
    return globalMaxMinLength;
}

float retrogradeAnalysisEminusL(uint64_t cluster)
{
    auto start = std::chrono::high_resolution_clock::now();

    // Add the red car line.
    int* unranked = new int[12];
    auto temp = unrank_fun<uint64_t>(cluster, 6, 11);
    std::copy(temp, temp+8, unranked);
    unranked[8] = 4;
    std::copy(temp+8, temp+11, unranked+9);
    uint64_t true_rank = rank_fun<uint64_t>(unranked, 6, 12);
    delete [] temp;

    // Evaluate if cluster is feasible.
    if(!checkCardinality(unranked))
    {
        delete [] unranked;
        return std::numeric_limits<float>::infinity();
    }

    // Prepare solver variables.
    piecesThreebyOneHorizontal.clear();
    piecesTwobyOneHorizontal.clear();
    piecesThreebyOneVertical.clear();
    piecesTwobyOneVertical.clear();

    int indexTh=0;
    int indexTw=0;

    for(int v = 0; v < 6; v++) // Verticals
    {
       switch(unranked[v])
       {
           case 1: // a, 3x1
               piecesThreebyOneVertical.push_back(static_cast<char>(threeOne[indexTh++])); 
           break;

           case 2: // b, c, 3x1, 2x1
           case 3:
               piecesThreebyOneVertical.push_back(static_cast<char>(threeOne[indexTh++]));
               piecesTwobyOneVertical.push_back(static_cast<char>(twoOne[indexTw++]));
           break;

           case 4: // d, 2x1
               piecesTwobyOneVertical.push_back(static_cast<char>(twoOne[indexTw++])); 
           break;

           case 5: // e, 2x1, 2x1
               piecesTwobyOneVertical.push_back(static_cast<char>(twoOne[indexTw++]));
               piecesTwobyOneVertical.push_back(static_cast<char>(twoOne[indexTw++]));
           break;
       }
    }

    for(int h = 6; h < 12; h++) // Horizontals
    {
        if(h == 8) continue; // x
        switch(unranked[h])
        {
            case 1: // a, 3x1
                piecesThreebyOneHorizontal.push_back(static_cast<char>(threeOne[indexTh++])); 
            break;

            case 2: // b, c, 3x1, 2x1
            case 3:
                piecesThreebyOneHorizontal.push_back(static_cast<char>(threeOne[indexTh++]));
                piecesTwobyOneHorizontal.push_back(static_cast<char>(twoOne[indexTw++]));
            break;

            case 4: // d, 2x1
                piecesTwobyOneHorizontal.push_back(static_cast<char>(twoOne[indexTw++])); 
            break;

            case 5: // e, 2x1, 2x1
                piecesTwobyOneHorizontal.push_back(static_cast<char>(twoOne[indexTw++])); 
                piecesTwobyOneHorizontal.push_back(static_cast<char>(twoOne[indexTw++])); 
            break;
        }
    }

    auto states = getPuzzleStates(unranked);

    std::unordered_map<State*, float> cumulativeEntropyMap;
    std::unordered_map<State*, float> cumulativeLengthMap;
    std::unordered_set<State*> open;
    
    bool hasSolution = false;

    for(const auto& state : states)
    {
        State* s = new State(state->ToStringSolvable(), true);
        if(s->isOneMoveFromGoal() || s->isGoal()){
            cumulativeEntropyMap[s] = 0.0f;
            cumulativeLengthMap[s] = 1.0f;
            hasSolution = true;
        }
        else{
            cumulativeEntropyMap[s] = std::numeric_limits<float>::infinity();
            cumulativeLengthMap[s] = std::numeric_limits<float>::infinity();
            open.insert(s);
        }
        delete state;
    }

    if(!hasSolution){
        for(const auto& entry : cumulativeEntropyMap){
            delete entry.first;
        }
        cumulativeEntropyMap.clear();
        cumulativeLengthMap.clear();
        delete [] unranked;
        return std::numeric_limits<float>::infinity();
    }
    else
    {
        bool mapsChanged = true;
        while(mapsChanged)
        {
            mapsChanged = false;
            for(auto it = open.begin(); it != open.end(); it++)
            {
                float currentEntropy = cumulativeEntropyMap[*it]; 
                float currentLength = cumulativeLengthMap[*it];
                float localEntropy = calculateStateUniformEntropy(*it);
                float localLength = 1.0f;

                auto children = (*it)->genSuccesors();
                float minChildEntropy = std::numeric_limits<float>::infinity();
                float minChildLength = std::numeric_limits<float>::infinity();

                for(const auto& child : children) 
                {
                    float childPathEntropy = cumulativeEntropyMap[child];
                    float childPathLength = cumulativeLengthMap[child];
                    
                    if(childPathEntropy != std::numeric_limits<float>::infinity()){
                        float totalEntropy = (localEntropy + childPathEntropy) * discountFactor; // discount factor
                        if(totalEntropy < minChildEntropy)
                        {
                            minChildEntropy = totalEntropy;
                        }
                    }

                    if(childPathLength != std::numeric_limits<float>::infinity()){
                        float totalLength = localLength + childPathLength;
                        if(totalLength < minChildLength) {
                            minChildLength = totalLength;
                        }
                    }

                    delete child;
                }
                
                if(minChildEntropy < currentEntropy)
                {
                    cumulativeEntropyMap[*it] = minChildEntropy;
                    mapsChanged = true;
                }

                if(minChildLength < currentLength) {
                    cumulativeLengthMap[*it] = minChildLength;
                    mapsChanged = true;
                }
            }
        }

        float maxEntropyInCluster = 0.0f;
        float maxLengthInCluster = 0.0f;
        
        for(auto it = open.begin(); it != open.end(); it++)
        {
            float e = cumulativeEntropyMap[*it];
            float l = cumulativeLengthMap[*it];

            if(e != std::numeric_limits<float>::infinity() && e > maxEntropyInCluster){
                maxEntropyInCluster = e;
            }

            if(l != std::numeric_limits<float>::infinity() && l > maxLengthInCluster){
                maxLengthInCluster = l;
            }
        }

        if(maxEntropyInCluster == 0.0f) maxEntropyInCluster = 1.0f;
        if(maxLengthInCluster == 0.0f) maxLengthInCluster = 1.0f;

        float maxNormalizedDiff = -std::numeric_limits<float>::infinity();
        std::vector<std::pair<State*, float>> sortedEntries;
        for(const auto & entry : cumulativeEntropyMap)
        {
            State* s = entry.first;
            float e = entry.second;
            float l = cumulativeLengthMap[s];

            if(e != std::numeric_limits<float>::infinity() && l != std::numeric_limits<float>::infinity()){
                float normEntropy = e / maxEntropyInCluster;
                float normLength = l / maxLengthInCluster;
                float diff = normEntropy - normLength;
                if(diff > maxNormalizedDiff)
                {
                    maxNormalizedDiff = diff;
                }
                sortedEntries.push_back({s, diff});
            }
        }

        std::sort(sortedEntries.begin(), sortedEntries.end(), 
        [](const std::pair<State*, float>& a, const std::pair<State*, float>& b) {
                return a.second > b.second; // Descending.
            }
        );

        int boardsPrinted = 0;
        float lastPrintedMetric = -std::numeric_limits<float>::infinity();
        
        for (const auto& entry : sortedEntries) {
            if(boardsPrinted >= printedBoardLimit){
                break;
            }
            if(entry.second != lastPrintedMetric){
                std::cout << "==============" << std::endl;
                std::cout << "Cluster: 0x" << std::hex << true_rank << " (converted to 0x" << cluster << std::dec << ")\n";
                std::cout << "Entropy: " << cumulativeEntropyMap[entry.first] << " | Length: " << cumulativeLengthMap[entry.first] << "\n";
            }
            lastPrintedMetric = entry.second;
            std::cout << "State: \n" << entry.first->toString() << std::endl;
            boardsPrinted++;
        }

        for(const auto& entry : cumulativeEntropyMap){
            delete entry.first;
        }
        cumulativeEntropyMap.clear();
        cumulativeLengthMap.clear();
        open.clear();

        delete [] unranked;
        return maxNormalizedDiff;
    }
}

int main(int argc, char** argv)
{

    std::string metric; 

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("file,f", po::value<std::string>(), "input file")
        ("algorithm,a", po::value<std::string>(&entropyAlgorithm)->default_value("retro"), "algorithm to use")
        ("iterative,i", "use iterative version of the algorithm")
        ("boards,b", po::value<int>(&printedBoardLimit)->default_value(25), " boards to print (descending order)")
        ("metric,m", po::value<std::string>(&metric)->default_value("entropy"), "metric to use (entropy, length or both).")
        ("discountfactor,d", po::value<float>(&discountFactor)->default_value(1.0f), "discount factor for entropy.")
        ;
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
    po::notify(vm);

    if(vm.count("help")) {
        std::cout << desc << "\n";
        return 0;
    }   

    _dummy_static_initializer dummy;
    initCols();

    if(vm.count("file")) {
        std::string filename = vm["file"].as<std::string>();
        std::ifstream infile(filename);
        if (!infile) {
            std::cerr << "Error opening file: " << filename << std::endl;
            return 1;
        }

        while(!infile.eof()){
            visited.clear();
            path.clear();
            std::string line;
            if(!std::getline(infile, line)) 
                break;
            
            if(entropyAlgorithm == "retro")
            {
                uint64_t cluster = std::stoull(line, nullptr, 16);
                if(metric == "entropy"){
                    float entropy = retrogradeAnalysisEntropy(cluster);
                    std::cout << "Max Entropy: " << entropy << std::endl;
                }else if(metric == "length"){
                    float length = retrogradeAnalysisLength(cluster);
                    std::cout << "Max Length: " << length << std::endl;
                }else if(metric == "e-l"){
                    float eMinusL = retrogradeAnalysisEminusL(cluster);
                    std::cout << "Max Entropy - Length: " << eMinusL << std::endl;
                }
            }
        }
    }

    return 0;
}