#include "Evaluator.h"
#include "MoveGenerator.h"
#include <array>
#include <algorithm>

static int value(PieceType p){
    switch(p){
        case PieceType::PAWN:return 100;
        case PieceType::KNIGHT:return 320;
        case PieceType::BISHOP:return 330;
        case PieceType::ROOK:return 500;
        case PieceType::QUEEN:return 900;
        case PieceType::KING:return 20000;
        default:return 0;
    }
}

static const int pawn[64]={
0,0,0,0,0,0,0,0, 50,50,50,50,50,50,50,50, 10,10,20,30,30,20,10,10,
5,5,10,25,25,10,5,5, 0,0,0,20,20,0,0,0, 5,-5,-10,0,0,-10,-5,5,
5,10,10,-20,-20,10,10,5, 0,0,0,0,0,0,0,0};
static const int knight[64]={
-50,-40,-30,-30,-30,-30,-40,-50,-40,-20,0,0,0,0,-20,-40,-30,0,10,15,15,10,0,-30,
-30,5,15,20,20,15,5,-30,-30,0,15,20,20,15,0,-30,-30,5,10,15,15,10,5,-30,
-40,-20,0,5,5,0,-20,-40,-50,-40,-30,-30,-30,-30,-40,-50};
static const int bishop[64]={
-20,-10,-10,-10,-10,-10,-10,-20,-10,0,0,0,0,0,0,-10,-10,0,5,10,10,5,0,-10,
-10,5,5,10,10,5,5,-10,-10,0,10,10,10,10,0,-10,-10,10,10,10,10,10,10,-10,
-10,5,0,0,0,0,5,-10,-20,-10,-10,-10,-10,-10,-10,-20};
static const int rook[64]={
0,0,0,0,0,0,0,0,5,10,10,10,10,10,10,5,-5,0,0,0,0,0,0,-5,-5,0,0,0,0,0,0,-5,
-5,0,0,0,0,0,0,-5,-5,0,0,0,0,0,0,-5, -5,0,0,0,0,0,0,-5,0,0,0,5,5,0,0,0};
static const int queen[64]={
-20,-10,-10,-5,-5,-10,-10,-20,-10,0,0,0,0,0,0,-10,-10,0,5,5,5,5,0,-10,
-5,0,5,5,5,5,0,-5,0,0,5,5,5,5,0,-5,-10,5,5,5,5,5,0,-10,-10,0,5,0,0,0,0,-10,
-20,-10,-10,-5,-5,-10,-10,-20};
static const int king[64]={
-30,-40,-40,-50,-50,-40,-40,-30,-30,-40,-40,-50,-50,-40,-40,-30,-30,-40,-40,-50,-50,-40,-40,-30,
-20,-30,-30,-40,-40,-30,-30,-20,-10,-20,-20,-20,-20,-20,-20,-10,20,20,0,0,0,0,20,20,
20,30,10,0,0,10,30,20};

static int pst(PieceType p,int sq){
    switch(p){case PieceType::PAWN:return pawn[sq];case PieceType::KNIGHT:return knight[sq];case PieceType::BISHOP:return bishop[sq];case PieceType::ROOK:return rook[sq];case PieceType::QUEEN:return queen[sq];case PieceType::KING:return king[sq];default:return 0;}
}

int Evaluator::material(const Board& b) const{
    int s=0;for(int r=0;r<8;r++)for(int c=0;c<8;c++){Piece p=b.getPiece(r,c);int v=value(p.type);s+=(p.color==Color::WHITE?v:-v);}return s;
}
int Evaluator::pieceSquare(const Board& b) const{
    int s=0;for(int r=0;r<8;r++)for(int c=0;c<8;c++){Piece p=b.getPiece(r,c);if(p.type==PieceType::NONE)continue;int sq=p.color==Color::WHITE?r*8+c:(7-r)*8+c;int v=pst(p.type,sq);s+=(p.color==Color::WHITE?v:-v);}return s;
}
int Evaluator::mobility(Board& b) const{
    MoveGenerator mg; Color side=b.getSideToMove(); int own=(int)mg.generateLegalMoves(b).size(); b.setSideToMove(opposite(side)); int other=(int)mg.generateLegalMoves(b).size(); b.setSideToMove(side); return (side==Color::WHITE?own-other:other-own)*2;
}
int Evaluator::evaluate(const Board& b) const{
    return material(b)+pieceSquare(b);
}
