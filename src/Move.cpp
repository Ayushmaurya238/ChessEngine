#include "Move.h"
#include <cctype>

std::string squareToString(int row, int col) {
    std::string s;
    s += char('a' + col);
    s += char('8' - row);
    return s;
}

std::string moveToUCI(const Move& move) {
    std::string s = squareToString(move.fromRow, move.fromCol) +
                    squareToString(move.toRow, move.toCol);
    if (move.promotionPiece != PieceType::NONE) {
        char p = 'q';
        if (move.promotionPiece == PieceType::ROOK) p = 'r';
        else if (move.promotionPiece == PieceType::BISHOP) p = 'b';
        else if (move.promotionPiece == PieceType::KNIGHT) p = 'n';
        s += p;
    }
    return s;
}

std::string moveToString(const Move& move) {
    return moveToUCI(move);
}
