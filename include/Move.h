#ifndef CHESS_ENGINE_MOVE_H
#define CHESS_ENGINE_MOVE_H

#include "Piece.h"
#include <string>
//  this contain the move intial pos to final position , if there is promotion or not , whether this moves do any capture or its a castling move or enpassant i.e. different types of move that could be 
struct Move {
    int fromRow = 0, fromCol = 0;
    int toRow = 0, toCol = 0;
    PieceType promotionPiece = PieceType::NONE;
    bool isCapture = false;
    bool isCastling = false;
    bool isEnPassant = false;
    
    Move() = default;
    // used initialisation list to build this 
    Move(int fr, int fc, int tr, int tc) :
        fromRow(fr), fromCol(fc), toRow(tr), toCol(tc) {}
    //  operator comparision overloaded to see whether the moves are same 
    bool operator==(const Move& o) const {
        return fromRow==o.fromRow && fromCol==o.fromCol &&
               toRow==o.toRow && toCol==o.toCol &&
               promotionPiece==o.promotionPiece &&
               isCastling==o.isCastling && isEnPassant==o.isEnPassant;
    }
};

std::string squareToString(int row, int col);
std::string moveToUCI(const Move& move);
std::string moveToString(const Move& move);

#endif
