#include <iostream>
#include <fstream>
#include <unordered_set>
#include <boost/program_options.hpp>

#include "Puzzle.h"
#include "PuzzleState.h"
#include "Utils.h"
#include "Common.h"

namespace po = boost::program_options;

int timeArg=86400,memoryArg=1000000,ttSizeArg=10000000,maxSolLength=1000;
std::string algorithmArg="Astar",heuristicArg="obstacles0";
HEURISTIC heuristic=obstacles0;
bool INCLUDE_FIELDER=false;

std::unordered_set<PuzzleState*> getStates(int* ur) {
    int dims = 12;
    std::unordered_set<PuzzleState*> states;
    std::vector<int> board(dims);

    for (int d = 0; d < dims; d++){
        board[d] = maxRows[ur[d]] - 1;
    }

    while(true)
    {
        PuzzleState* state = PuzzleState::makeState(board.data() + 6, board.data());
        if(state != nullptr)
            states.insert(state);

        int i = dims - 1;
        while(i >= 0)
        {
            board[i]--;
            if(board[i] >= minRows[ur[i]])
                break;
            else
            {
                board[i] = maxRows[ur[i]] -1;
                i--;
            }
        }
        if(i < 0) break;
    }
    return states;
}

bool checkCardinality(int* b, bool c_or_b)
{
    int three_by_ones = 0;
    int two_by_ones = 0; 
    for(int i = 0; i < 12; i++)
    {
        if((!c_or_b && b[i] > 4 && b[i] <= 4) || (c_or_b && b[i] == 1)){
            three_by_ones+=1;
        }
        else if((!c_or_b && b[i] > 4 && b[i] < 11) || (c_or_b && (b[i] == 2 || b[i] == 3))){
            three_by_ones+=1;
            two_by_ones+=1;
        }
        else if((!c_or_b && b[i] >= 11 && b[i] < 16) || (c_or_b && b[i] == 4)){
            two_by_ones+=1;
        }
        else if((!c_or_b && b[i] >= 16) || (c_or_b && b[i] == 5)){
            two_by_ones+=2;
        }
    }

    return three_by_ones <= 4 && two_by_ones <= 12;
}

void identify_cc(uint64_t cluster) {
    int* true_ur = new int[12];
    int* ur = unrank_fun<uint64_t>(cluster, 6, 11);
    std::copy(ur, ur+8, true_ur);
    true_ur[8] = 4;
    std::copy(ur+8, ur+11, true_ur+9);
    delete [] ur;
    
    uint64_t true_r = rank_fun<uint64_t>(true_ur, 6, 12);
    
    if(!checkCardinality(true_ur, true))
    {
        // std::cout << "Not solvable due to cardinality constraints." << std::endl;
        delete [] true_ur;
        return;
    }

    // Needed for solver to not explode.
    piecesThreebyOneHorizontal.clear();
    piecesThreebyOneVertical.clear();
    piecesTwobyOneHorizontal.clear();
    piecesTwobyOneVertical.clear();

    int indexTh = 0;
    int indexTw = 0;

    for(int v = 0; v < 6; v++)
    {
        switch(true_ur[v])
        {
            case 1: // a 3x1 vertical
                piecesThreebyOneVertical.push_back(static_cast<char>(threeOne[indexTh++]));
                break;
            case 2:
            case 3: // b, c, 3x1, 2x1
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

    for(int h = 6; h < 12; h++)
    {
        if(h == 8) continue; // x
        switch(true_ur[h])
        {
            case 1: // a 3x1 horizontal
                piecesThreebyOneHorizontal.push_back(static_cast<char>(threeOne[indexTh++]));
                break;
            case 2:
            case 3: // b, c, 3x1, 2x1
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

    // Get states.
    auto states = getStates(true_ur);
    // std::cout << states.size() << std::endl;
    // std::cout << "Cluster: " << std::hex << cluster << " True Rank : " << std::hex << true_r << std::dec << std::endl;  
    // std::cout << "Number of states: " << states.size() << std::endl;
    
    

    // Run BFS on all states in the cluster.
    for(const auto& state : states) {
        Puzzle p("", state->ToStringSolvable(), true);  
        p.init();
        p.BFS(-1); 
        delete state;
    }

    // Cluster;Size;#ConnectedComponent;#SolvableCC;LargestCC;SmallestCC;LargestSolvableCC;SmallestSolvableCC;
    
    std::stringstream data;
    data << "0x" << std::hex << true_r << std::dec << " (transformed from 0x" << std::hex << cluster << std::dec << ")\n";
    data << std::dec << "Cluster Size: " << states.size() << "\n";
    // data << std::hex << "0x" << true_r << ",";
    // data << std::dec << states.size() << ",";
    for(size_t i = 0; i < Puzzle::TTs.size(); ++i) {
        const auto& tt = Puzzle::TTs[i];
        data << "Connected component (CC) #" << i << "\n";
        data << "CC Size: " << tt->size() << "\n";
        data << "CC Solvable: " << tt->isSolvable() << "\n";
        if(tt->isSolvable())
            data << "CC Solution Length. " << tt->getMaxSolutionLength() << "\n";
        data << "CC Radius: " << tt->getRadius() << "\n";
        data << "CC Diameter: " << tt->getDiameter() << "\n";
        data << "CC Branching Factor: " << tt->getBranchingFactor() << "\n";
        data << "CC Avg. Cluster Coefficient: " << tt->getLocalCCSum()/tt->size() << "\n";
        if(INCLUDE_FIELDER)
        {
            data << "CC Fielder Value: " << tt->getFiedlerValue() << "\n";
        } 
        data << "========" << "\n"; 
        // data << tt->statistics();
        // data << tt->size() << ",";
        // data << tt->isSolvable() << ",";
        // data << tt->getRadius() << ",";
        // // std::cout << tt->getFirst()->toString() << std::endl;
        // data << tt->getMaxSolutionLength() << ",";
        // data << tt->getDiameter() << ",";
        // data << tt->getBranchingFactor() << ",";
        // data << tt->getLocalCCSum()/tt->size();
        
        // if(i < Puzzle::TTs.size() - 1){
        //     data << ",";
        // }
        // std::cout << tt->getFiedlerValue() << std::endl;

        // if(tt->size() == 706){
        // for (const auto& [u, neighbors] : tt->adj) {
        //     for (const auto* v : neighbors) {
        //         if (u < v) { // Print each undirected edge only once
        //             std::cout << u << " " << v << std::endl;
        //             }
        //         }
        //     }
        // } 

        delete tt;
    }

    std::cout << data.str() << std::endl;

    Puzzle::TTs.clear();
    states.clear();
    delete [] true_ur;
}


int main(int argc, char* argv[]) {
    po::options_description desc("Allowed options");

    desc.add_options()
        ("help,h", "produce help message")
        ("file,f", po::value<std::string>(), "input file")
        ("output,o", po::value<std::string>(), "output file")
        ("fiedler", "include fiedler value in calculation");


    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
    po::notify(vm);

    if(vm.count("help")) {
        std::cout << desc << "\n";
        return 0;
    }

    _dummy_static_initializer dummy;
    initCols();

    if(vm.count("fiedler")){
        INCLUDE_FIELDER = true;
    }
    
    if(vm.count("file")) {
        std::string filename = vm["file"].as<std::string>();
        std::ifstream input(filename);
        if (!input.is_open()) {
            std::cerr << "Error opening file: " << filename << std::endl;
            return 1;
        }

        while(!input.eof()) {
            std::string line;
            if (!std::getline(input, line)) {
                break; // End of file
            }
            // Process the line as needed
            uint64_t cluster = std::stoull(line, nullptr, 16);
            identify_cc(cluster);

        }
    }   

    return 0;
}