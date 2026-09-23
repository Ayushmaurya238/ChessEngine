#include "ChessEngine.h"
#include "FENParser.h"
#include "Move.h"
#include <sstream>
#include <algorithm>

void ChessEngine::newGame(){board.initialize();searchEngine.clear();}
bool ChessEngine::loadFEN(const std::string& fen){
    try{board=FENParser::fromFEN(fen);searchEngine.clear();return true;}catch(...){return false;}
}
std::vector<Move> ChessEngine::legalMoves(){return moveGenerator.generateLegalMoves(board);}
bool ChessEngine::makeMove(const Move& move){
    auto moves=legalMoves();
    for(const auto&m:moves)if(m==move){board.makeMove(m);return true;}
    return false;
}
bool ChessEngine::undoMove(){ if(!board.canUndo()) return false; board.undoMove(); return true; }

bool ChessEngine::makeUCIMove(const std::string& uci){
    if(uci.size()<4)return false;int fc=uci[0]-'a',fr=8-(uci[1]-'0'),tc=uci[2]-'a',tr=8-(uci[3]-'0');
    PieceType promo=PieceType::NONE;
    if(uci.size()>=5){switch(uci[4]){case'q':promo=PieceType::QUEEN;break;case'r':promo=PieceType::ROOK;break;case'b':promo=PieceType::BISHOP;break;case'n':promo=PieceType::KNIGHT;break;}}
    for(auto&m:legalMoves())if(m.fromRow==fr&&m.fromCol==fc&&m.toRow==tr&&m.toCol==tc&&m.promotionPiece==promo){board.makeMove(m);return true;}
    return false;
}
Move ChessEngine::getBestMove(int depth,int timeMs){return searchEngine.findBestMove(board,depth,timeMs);}
void ChessEngine::printBoard() const{board.print();}
std::string ChessEngine::getFEN() const{return board.toFEN();}
std::string ChessEngine::movesToString(const std::vector<Move>& moves){
    std::ostringstream os;for(size_t i=0;i<moves.size();i++){if(i)os<<' ';os<<moveToUCI(moves[i]);}return os.str();
}
