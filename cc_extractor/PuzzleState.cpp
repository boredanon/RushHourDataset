#include "PuzzleState.h"

bool PuzzleState::addRow(int rowType, int rowPos){
   std::bitset<36> allPieces(piecesOneThree|piecesOneTwo);
    
    if((allPieces & (rowBits[rowType]<< (rowPos*6))).any() ){
        return false;//collision
    }
    piecesOneThree|=(rowBitsOneThree[rowType]<< (rowPos*6));//add 3x1 pieces
    piecesOneTwo|=(rowBitsOneTwo[rowType]<< (rowPos*6));//add 2x1 pieces
    return true;
}

bool PuzzleState::addCol(int colType, int colPos)
{
    std::bitset<36> allPieces(piecesOneThree|piecesOneTwo);
    if((allPieces & (colBits[colType] << (colPos))).any()){
        return false;
    }
    piecesOneThree |= (colBitsOneThree[colType] << (colPos));
    piecesOneTwo |= (colBitsOneTwo[colType] << (colPos));
    vert |= (colBitsOneTwo[colType] << (colPos));
    vert |=(colBitsOneThree[colType] << (colPos));
    return true;
}

PuzzleState* PuzzleState::makeState(int rows[7],int cols[7])
{
    PuzzleState *state=new PuzzleState;

    for(int c=0;c<6;c++){//for each row
        if(!state->addCol(cols[c],c)){ // First let's add cols (right to left)
            delete state;
            return nullptr;
        }

        if(rows != nullptr){
        //todo: add cols
        //todo:add vert and rover bits
            if(!state->addRow(rows[c],c)){ // Add rows (bottom to top.)
                delete state;
                return nullptr;
            }

        }

    }
    return state;
}

// bool PuzzleState::addTwoByTwoPiece(int pos, bool isRover){
//     assert(pos>=0 && pos<48 && (pos%7)!=6);//check valid pos
//     bitset<49> allPieces(piecesOneThree|piecesOneTwo|piecesTwoTwo);

//     bitset<49> rover;
//     rover[pos]=rover[pos+1]=rover[pos+7]=rover[pos+8]=1;
//     if((allPieces & rover).any() ){
//         //cout<<"Illegal colision placing rover"<<endl;
//         return false;//collision
//     }

//     piecesTwoTwo|=rover;
//     if(isRover) vertOrRover|=rover;
//     return true;
// }

void PuzzleState::print() const{

    std::bitset<36> allPieces(piecesOneThree|piecesOneTwo);
    for(int i=0;i<36;i++){
        std::cout<<allPieces[i];
        if((i+1)%6==0)
            std::cout<<std::endl;
    }
    std::cout<<std::endl;
}

std::string PuzzleState::ToStringSolvable() const
{
    std::string ss(36, '.');
    int indexTh[2] = {0,0};
    int indexTw[2] = {0,0};

    for(int col = 0; col < 6; col++)
    {
        for(int row = 0; row < 6; row++)
        {
            if(ss[row * 6 + col] != '.') continue;
            if(piecesOneThree[row * 6 + col] && vert[row * 6 + col])
            {
                //b or c.
                ss[row * 6 + col] = piecesThreebyOneVertical[indexTh[0]];
                ss[(row+1) * 6 + col] = piecesThreebyOneVertical[indexTh[0]];
                ss[(row+2) * 6 + col] = piecesThreebyOneVertical[indexTh[0]];
                indexTh[0]++;
            }
            if(piecesOneTwo[row * 6 + col] && vert[row * 6 + col])
            {
                //a, d ,e
                ss[row * 6 + col] = piecesTwobyOneVertical[indexTw[0]];
                ss[(row+1) * 6 + col] = piecesTwobyOneVertical[indexTw[0]];
                indexTw[0]++;
            }            
        }
    }

    for(int row = 0; row < 6; row++)
    {
        for(int col = 0; col < 6; col++)
        {
            if(ss[row * 6 + col] != '.') continue;
            if(piecesOneThree[row * 6 + col] && !vert[row * 6 + col])
            {
                ss[row * 6 + col] = piecesThreebyOneHorizontal[indexTh[1]];
                ss[row * 6 + (col+1)] = piecesThreebyOneHorizontal[indexTh[1]];
                ss[row * 6 + (col+2)] = piecesThreebyOneHorizontal[indexTh[1]];
                indexTh[1]++;
            }
            if(piecesOneTwo[row * 6 + col] && !vert[row * 6 + col])
            {
                if(row != 2)
                {
                    ss[row * 6 + col] = piecesTwobyOneHorizontal[indexTw[1]];
                    ss[row * 6 + (col + 1)] = piecesTwobyOneHorizontal[indexTw[1]];
                    indexTw[1]++;
                }
                else
                {
                    ss[row * 6 + col] = 'x';
                    ss[row * 6 + (col+1)] = 'x';
                }
            }
        }
    }
    // std::queue<char> q_3x1;
    // std::queue<char> q_2x1;
    // std::queue<char> q_2x2;

    // q_3x1.push('o'); 
    // q_3x1.push('p'); 
    // q_3x1.push('q'); 
    // q_3x1.push('r'); 
    // q_3x1.push('s'); 


    // q_2x1.push('a');
    // q_2x1.push('f');
    // q_2x1.push('d');
    // q_2x1.push('g');
    // q_2x1.push('j');
    // q_2x1.push('b');
    // q_2x1.push('h');
    // q_2x1.push('i');
    // q_2x1.push('k');
    // q_2x1.push('c');
    // q_2x1.push('e');


    // q_2x2.push('u');
    // q_2x2.push('v');

    // bool used[20]; // Has the piece letter been used or not. (piece_letter in Common.cpp) 
    // int indexTh[2] = {0,0};
    // int indexTw[2] = {0,0};
    // for(int i = 0; i < 20; i++) used[i] = false;

    // for(int col = 0; col < 7; col++)
    // {
    //     for(int row = 0; row < 7; row++)
    //     {
    //         if(ss[row * 7 + col] != '.') continue; // Don't double count pieces.
    //         if(piecesOneThree[row * 7 + col] & vertOrRover[row * 7 + col])
    //         {
    //             auto p = q_3x1.front();
    //             q_3x1.pop();
    //             //b or c.
    //             ss[row * 7 + col] = p;//piecesThreebyOneVertical[indexTh[0]];
    //             ss[(row+1) * 7 + col] = p;//piecesThreebyOneVertical[indexTh[0]];
    //             ss[(row+2) * 7 + col] = p;//piecesThreebyOneVertical[indexTh[0]];
    //             indexTh[0]++;
    //         }
    //         if(piecesOneTwo[row * 7 + col] & vertOrRover[row * 7 + col])
    //         {
    //             auto p = q_2x1.front();
    //             q_2x1.pop();
    //             //a, d, e
    //             ss[row * 7 + col] = p;//piecesTwobyOneVertical[indexTw[0]];
    //             ss[(row+1) * 7 + col] = p;//piecesTwobyOneVertical[indexTw[0]];
    //             indexTw[0]++;
    //         }

    //         if(piecesTwoTwo[row * 7 + col]) // u, v, x (rover)
    //         {
    //             if(vertOrRover[row * 7 + col])
    //             {
    //                 ss[row * 7 + col] = 'x';//solver_piece_letter[3]; // x
    //                 ss[(row+1) * 7 + col] = 'x';//solver_piece_letter[3];
    //                 ss[row * 7 + (col+1)] = 'x';solver_piece_letter[3];
    //                 ss[(row+1)*7+(col+1)] = 'x';solver_piece_letter[3];
    //             }
    //             else
    //             {
    //                 for(int j = 1; j < 3; j++) // indices for 2x2 pieces (except rover)
    //                 {
    //                     if(!used[j])
    //                     {   
    //                         ss[row * 7 + col] = solver_piece_letter[j];
    //                         ss[(row+1) * 7 + col] = solver_piece_letter[j];
    //                         ss[row * 7 + (col+1)] = solver_piece_letter[j];
    //                         ss[(row+1)*7+(col+1)] = solver_piece_letter[j];

    //                         used[j] = true;
    //                         break;
    //                     }
                        
    //                 }
    //             }
    //         }
    //     }
    // }


    

    // for(int row = 0; row < 7; row++)
    // {
    //     for(int col = 0; col < 7; col++)
    //     {
    //         if(ss[row * 7 + col] != '.') continue; // Don't double count pieces.
    //         if(piecesOneThree[row * 7 + col] &!vertOrRover[row * 7 + col])
    //         {
    //             auto p = q_3x1.front();
    //             q_3x1.pop();
    //             ss[row * 7 + col] = p;//piecesThreebyOneHorizontal[indexTh[1]];
    //             ss[(row) * 7 + (col+1)] = p;//piecesThreebyOneHorizontal[indexTh[1]];
    //             ss[(row) * 7 + (col+2)] = p;//piecesThreebyOneHorizontal[indexTh[1]];
    //             indexTh[1]++;
    //         }
    //         if(piecesOneTwo[row * 7 + col] & !vertOrRover[row * 7 + col])
    //         {
    //             auto p = q_2x1.front();
    //             q_2x1.pop();
    //             //a, d, 
    //             ss[row * 7 + col] = p;//piecesTwobyOneHorizontal[indexTw[1]];
    //             ss[(row) * 7 + (col+1)] = p; //piecesTwobyOneHorizontal[indexTw[1]];
    //             indexTw[1]++;   
    //         }
    //     }
    // }
    // /*indexTh[0] = 0;
    // indexTh[1] = 0;
    // indexTw[0] = 0;
    // indexTw[1] = 0;
    

    // for(int i = 0; i < 49; i++)
    // {
    //     if(s[i] != '.') continue; // Space has been occupied.

    //     if(piecesOneThree[i]){ // o, p, q, r, s
    //        if(vertOrRover[i] && i+7 < 49 && i+14 < 49) // Vertical
    //         {
    //             s[i] = piecesThreebyOneVertical[indexTh[0]];
    //             s[i+7] = piecesThreebyOneVertical[indexTh[0]];
    //             s[i+14] = piecesThreebyOneVertical[indexTh[0]];
    //             indexTh[0]++;
    //         }
    //         else// Horizontal
    //         {
    //             s[i] = piecesThreebyOneHorizontal[indexTh[1]];
    //             s[i+1] = piecesThreebyOneHorizontal[indexTh[1]];
    //             s[i+2] = piecesThreebyOneHorizontal[indexTh[1]];
    //             indexTh[1]++;
    //         }
    //     }

    //     if(piecesOneTwo[i]) // a, f, d, g, j, b, h, i, k, c, e
    //     {
    //         if(vertOrRover[i] && i+7 < 49)
    //         {
    //             s[i] = piecesTwobyOneVertical[indexTw[0]];
    //             s[i+7] = piecesTwobyOneVertical[indexTw[0]];
    //             indexTw[0]++;
    //         }
    //         else
    //         {
    //             s[i] = piecesTwobyOneHorizontal[indexTw[1]];
    //             s[i+1] = piecesTwobyOneHorizontal[indexTw[1]];
    //             indexTw[1]++;
    //         }
    //     }

    //     if(piecesTwoTwo[i]) // u, v, x (rover)
    //     {
    //         if(vertOrRover[i])
    //         {
    //             s[i] = solver_piece_letter[3]; // x
    //             s[i+7] = solver_piece_letter[3];
    //             s[i+1] = solver_piece_letter[3];
    //             s[i+8] = solver_piece_letter[3];
    //         }
    //         else
    //         {
    //             for(int j = 1; j < 3; j++) // indices for 2x2 pieces (except rover)
    //             {
    //                 if(!used[j])
    //                 {   
    //                     s[i] = solver_piece_letter[j];
    //                     s[i+7] = solver_piece_letter[j];
    //                     s[i+1] = solver_piece_letter[j];
    //                     s[i+8] = solver_piece_letter[j];

    //                     used[j] = true;
    //                     break;
    //                 }
                    
    //             }
    //         }
    //     }

    // }*/


    return ss;
}