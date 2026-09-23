#include "UCI.h"
#include "Move.h"
#include <iostream>
#include <sstream>

void UCI::handlePosition(const std::string& command){
    std::istringstream ss(command);std::string token;ss>>token;
    ss>>token;
    if(token=="startpos")engine.newGame();
    else if(token=="fen"){
        std::string fen,part;
        for(int i=0;i<6&&ss>>part;i++){if(i)fen+=' ';fen+=part;}
        engine.loadFEN(fen);
    }
    while(ss>>token)if(token=="moves")while(ss>>token)engine.makeUCIMove(token);
}
void UCI::handleGo(const std::string& command){
    std::istringstream ss(command);std::string t;int depth=4,movetime=0;
    while(ss>>t){
        if(t=="depth")ss>>depth;
        else if(t=="movetime")ss>>movetime;
    }
    Move m=engine.getBestMove(depth,movetime);
    std::cout<<"bestmove "<<moveToUCI(m)<<std::endl;
}
void UCI::run(){
    std::string line;
    while(std::getline(std::cin,line)){
        if(line=="uci"){std::cout<<"id name CppChessEngine\nid author Ayush\nuciok\n";}
        else if(line=="isready")std::cout<<"readyok\n";
        else if(line=="ucinewgame")engine.newGame();
        else if(line.rfind("position",0)==0)handlePosition(line);
        else if(line.rfind("go",0)==0)handleGo(line);
        else if(line=="stop"){}
        else if(line=="quit")break;
        else if(line=="d")engine.printBoard();
    }
}
