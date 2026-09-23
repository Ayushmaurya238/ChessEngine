#include "MoveGenerator.h"
#include <algorithm>
#include <array>

static bool onBoard(int r,int c){ return r>=0&&r<8&&c>=0&&c<8; }

void MoveGenerator::generatePawnMoves(const Board& b,int r,int c,std::vector<Move>& out) const {
    Piece p=b.getPiece(r,c); int d=p.color==Color::WHITE?-1:1; int start=p.color==Color::WHITE?6:1;
    int promo=p.color==Color::WHITE?0:7;
    int nr=r+d;
    if(onBoard(nr,c)&&b.getPiece(nr,c).type==PieceType::NONE){
        if(nr==promo){
            for(auto pt:{PieceType::QUEEN,PieceType::ROOK,PieceType::BISHOP,PieceType::KNIGHT}){ Move m(r,c,nr,c);m.promotionPiece=pt;out.push_back(m);}
        } else out.emplace_back(r,c,nr,c);
        if(r==start && b.getPiece(r+2*d,c).type==PieceType::NONE) out.emplace_back(r,c,r+2*d,c);
    }
    for(int dc:{-1,1}){
        nr=r+d; int nc=c+dc; if(!onBoard(nr,nc)) continue;
        Piece t=b.getPiece(nr,nc);
        if(t.type!=PieceType::NONE && t.color!=p.color){
            if(nr==promo){for(auto pt:{PieceType::QUEEN,PieceType::ROOK,PieceType::BISHOP,PieceType::KNIGHT}){Move m(r,c,nr,nc);m.promotionPiece=pt;m.isCapture=true;out.push_back(m);}}
            else {Move m(r,c,nr,nc);m.isCapture=true;out.push_back(m);}
        }
        if(nr==b.getEnPassantRow() && nc==b.getEnPassantCol()){
            Piece ep=b.getPiece(r,nc);
            if(ep.type==PieceType::PAWN && ep.color!=p.color){Move m(r,c,nr,nc);m.isCapture=m.isEnPassant=true;out.push_back(m);}
        }
    }
}

void MoveGenerator::generateKnightMoves(const Board& b,int r,int c,std::vector<Move>& out) const {
    static const int d[8][2]={{1,2},{2,1},{2,-1},{1,-2},{-1,-2},{-2,-1},{-2,1},{-1,2}};
    Piece p=b.getPiece(r,c);
    for(auto& x:d){int nr=r+x[0],nc=c+x[1];if(!onBoard(nr,nc))continue;Piece t=b.getPiece(nr,nc);if(t.type==PieceType::NONE||t.color!=p.color){Move m(r,c,nr,nc);m.isCapture=t.type!=PieceType::NONE;out.push_back(m);}}
}

void MoveGenerator::generateBishopMoves(const Board& b,int r,int c,std::vector<Move>& out) const {
    static const int d[4][2]={{1,1},{1,-1},{-1,1},{-1,-1}}; Piece p=b.getPiece(r,c);
    for(auto& x:d){for(int nr=r+x[0],nc=c+x[1];onBoard(nr,nc);nr+=x[0],nc+=x[1]){Piece t=b.getPiece(nr,nc);if(t.type==PieceType::NONE)out.emplace_back(r,c,nr,nc);else{if(t.color!=p.color){Move m(r,c,nr,nc);m.isCapture=true;out.push_back(m);}break;}}}
}
void MoveGenerator::generateRookMoves(const Board& b,int r,int c,std::vector<Move>& out) const {
    static const int d[4][2]={{1,0},{-1,0},{0,1},{0,-1}}; Piece p=b.getPiece(r,c);
    for(auto& x:d){for(int nr=r+x[0],nc=c+x[1];onBoard(nr,nc);nr+=x[0],nc+=x[1]){Piece t=b.getPiece(nr,nc);if(t.type==PieceType::NONE)out.emplace_back(r,c,nr,nc);else{if(t.color!=p.color){Move m(r,c,nr,nc);m.isCapture=true;out.push_back(m);}break;}}}
}
void MoveGenerator::generateQueenMoves(const Board& b,int r,int c,std::vector<Move>& out) const {
    generateBishopMoves(b,r,c,out); generateRookMoves(b,r,c,out);
}

void MoveGenerator::generateKingMoves(const Board& b,int r,int c,std::vector<Move>& out) const {
    Piece p=b.getPiece(r,c);
    for(int dr=-1;dr<=1;dr++)for(int dc=-1;dc<=1;dc++){if(!dr&&!dc)continue;int nr=r+dr,nc=c+dc;if(!onBoard(nr,nc))continue;Piece t=b.getPiece(nr,nc);if(t.type==PieceType::NONE||t.color!=p.color){Move m(r,c,nr,nc);m.isCapture=t.type!=PieceType::NONE;out.push_back(m);}}
    Color enemy=opposite(p.color);
    int home=p.color==Color::WHITE?7:0;
    if(r==home&&c==4&&!isInCheck(b,p.color)){
        if(b.canCastleKingSide(p.color)&&b.getPiece(home,5).type==PieceType::NONE&&b.getPiece(home,6).type==PieceType::NONE&&
           b.getPiece(home,7).type==PieceType::ROOK&&b.getPiece(home,7).color==p.color&&
           !isSquareAttacked(b,home,5,enemy)&&!isSquareAttacked(b,home,6,enemy)){Move m(r,c,home,6);m.isCastling=true;out.push_back(m);}
        if(b.canCastleQueenSide(p.color)&&b.getPiece(home,1).type==PieceType::NONE&&b.getPiece(home,2).type==PieceType::NONE&&b.getPiece(home,3).type==PieceType::NONE&&
           b.getPiece(home,0).type==PieceType::ROOK&&b.getPiece(home,0).color==p.color&&
           !isSquareAttacked(b,home,3,enemy)&&!isSquareAttacked(b,home,2,enemy)){Move m(r,c,home,2);m.isCastling=true;out.push_back(m);}
    }
}

std::vector<Move> MoveGenerator::generatePseudoLegalMoves(const Board& b) const {
    std::vector<Move> out;
    Color side=b.getSideToMove();
    for(int r=0;r<8;r++)for(int c=0;c<8;c++){
        Piece p=b.getPiece(r,c); if(p.type==PieceType::NONE||p.color!=side)continue;
        switch(p.type){
            case PieceType::PAWN:generatePawnMoves(b,r,c,out);break;
            case PieceType::KNIGHT:generateKnightMoves(b,r,c,out);break;
            case PieceType::BISHOP:generateBishopMoves(b,r,c,out);break;
            case PieceType::ROOK:generateRookMoves(b,r,c,out);break;
            case PieceType::QUEEN:generateQueenMoves(b,r,c,out);break;
            case PieceType::KING:generateKingMoves(b,r,c,out);break;
            default:break;
        }
    }
    return out;
}

bool MoveGenerator::isSquareAttacked(const Board& b,int r,int c,Color by) {
    // Pawn
    int pr=r+(by==Color::WHITE?1:-1);
    for(int dc:{-1,1})if(onBoard(pr,c+dc)){Piece p=b.getPiece(pr,c+dc);if(p.type==PieceType::PAWN&&p.color==by)return true;}
    // Knight
    static const int nd[8][2]={{1,2},{2,1},{2,-1},{1,-2},{-1,-2},{-2,-1},{-2,1},{-1,2}};
    for(auto& d:nd){int nr=r+d[0],nc=c+d[1];if(onBoard(nr,nc)){Piece p=b.getPiece(nr,nc);if(p.type==PieceType::KNIGHT&&p.color==by)return true;}}
    // King
    for(int dr=-1;dr<=1;dr++)for(int dc=-1;dc<=1;dc++){if(!dr&&!dc)continue;int nr=r+dr,nc=c+dc;if(onBoard(nr,nc)){Piece p=b.getPiece(nr,nc);if(p.type==PieceType::KING&&p.color==by)return true;}}
    // Sliding
    static const int orth[4][2]={{1,0},{-1,0},{0,1},{0,-1}};
    for(auto& d:orth){for(int nr=r+d[0],nc=c+d[1];onBoard(nr,nc);nr+=d[0],nc+=d[1]){Piece p=b.getPiece(nr,nc);if(p.type!=PieceType::NONE){if(p.color==by&&(p.type==PieceType::ROOK||p.type==PieceType::QUEEN))return true;break;}}}
    static const int diag[4][2]={{1,1},{1,-1},{-1,1},{-1,-1}};
    for(auto& d:diag){for(int nr=r+d[0],nc=c+d[1];onBoard(nr,nc);nr+=d[0],nc+=d[1]){Piece p=b.getPiece(nr,nc);if(p.type!=PieceType::NONE){if(p.color==by&&(p.type==PieceType::BISHOP||p.type==PieceType::QUEEN))return true;break;}}}
    return false;
}

bool MoveGenerator::isInCheck(const Board& b,Color c) {
    int k=b.findKing(c); if(k<0)return false;
    return isSquareAttacked(b,k/8,k%8,opposite(c));
}

std::vector<Move> MoveGenerator::generateLegalMoves(Board& b) const {
    std::vector<Move> legal;
    auto pseudo=generatePseudoLegalMoves(b);
    Color side=b.getSideToMove();
    for(const Move& m:pseudo){
        b.makeMove(m);
        if(!isInCheck(b,side)) legal.push_back(m);
        b.undoMove();
    }
    return legal;
}
