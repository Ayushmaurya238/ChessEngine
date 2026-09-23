#include "ZobristHasher.h"
#include <random>

ZobristHasher::ZobristHasher(){ init(); }

void ZobristHasher::init(){
    std::mt19937_64 rng(0xC0FFEE123456789ULL);
    for(auto& color:pieceKeys)for(auto& piece:color)for(auto& x:piece)x=rng();
    sideKey=rng();for(auto& x:castleKeys)x=rng();for(auto& x:epKeys)x=rng();initialized=true;
}
uint64_t ZobristHasher::hash(const Board& b){
    if(!initialized)init();
    uint64_t h=0;
    for(int r=0;r<8;r++)for(int c=0;c<8;c++){
        Piece p=b.getPiece(r,c); if(p.type==PieceType::NONE)continue;
        int pt=(int)p.type-1; if(pt<0||pt>5)continue;
        h^=pieceKeys[colorIndex(p.color)][pt][r*8+c];
    }
    if(b.getSideToMove()==Color::BLACK)h^=sideKey;
    const BoardState& s=b.getState();
    int cr=(s.whiteKingSideCastle?1:0)|(s.whiteQueenSideCastle?2:0)|
           (s.blackKingSideCastle?4:0)|(s.blackQueenSideCastle?8:0);
    h^=castleKeys[cr];
    if(s.enPassantCol>=0)h^=epKeys[s.enPassantCol];
    return h;
}
