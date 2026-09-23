#include "ChessEngine.h"
#include "FENParser.h"
#include "Move.h"
#include "UCI.h"
#include "MoveGenerator.h"
#include <iostream>
#include <sstream>
#include <string>
#include <cstdint>

static uint64_t perft(Board& b, MoveGenerator& mg, int depth){
    if(depth==0)return 1;
    auto moves=mg.generateLegalMoves(b);
    uint64_t nodes=0;
    for(const auto&m:moves){b.makeMove(m);nodes+=perft(b,mg,depth-1);b.undoMove();}
    return nodes;
}
static void perftDivide(Board& b, MoveGenerator& mg,int depth){
    auto moves=mg.generateLegalMoves(b);uint64_t total=0;
    for(auto&m:moves){b.makeMove(m);uint64_t n=perft(b,mg,depth-1);b.undoMove();std::cout<<moveToUCI(m)<<": "<<n<<"\n";total+=n;}
    std::cout<<"Total: "<<total<<"\n";
}
static void printMenu(){
    std::cout<<"\n1. New Game\n2. Load FEN\n3. Print Board\n4. Make Move\n5. Engine Move\n6. Show Legal Moves\n7. Run Perft\n8. Show FEN\n9. UCI mode\n0. Exit\n> ";
}
int main(){
    ChessEngine engine;MoveGenerator mg;
    std::cout<<"Cpp Chess Engine\n";
    while(true){
        printMenu();int choice;if(!(std::cin>>choice))break;std::cin.ignore(10000,'\n');
        if(choice==0)break;
        if(choice==1){engine.newGame();engine.printBoard();}
        else if(choice==2){std::string fen;std::cout<<"FEN: ";std::getline(std::cin,fen);if(!engine.loadFEN(fen))std::cout<<"Invalid FEN\n";else engine.printBoard();}
        else if(choice==3)engine.printBoard();
        else if(choice==4){std::string m;std::cout<<"Move (UCI, e.g. e2e4): ";std::getline(std::cin,m);std::cout<<(engine.makeUCIMove(m)?"Move made":"Illegal move")<<"\n";}
        else if(choice==5){int depth=4;std::cout<<"Depth: ";std::cin>>depth;std::cin.ignore(10000,'\n');Move m=engine.getBestMove(depth);std::cout<<"Engine: "<<moveToUCI(m)<<"\n";engine.makeMove(m);engine.printBoard();}
        else if(choice==6){auto moves=engine.legalMoves();std::cout<<ChessEngine::movesToString(moves)<<"\nCount: "<<moves.size()<<"\n";}
        else if(choice==7){int d;std::cout<<"Perft depth: ";std::cin>>d;std::cin.ignore(10000,'\n');Board b=engine.getBoard();perftDivide(b,mg,d);}
        else if(choice==8)std::cout<<engine.getFEN()<<"\n";
        else if(choice==9){UCI uci;uci.run();break;}
    }
    return 0;
}
