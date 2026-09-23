#ifndef CHESS_ENGINE_MOVEGENERATOR_H
#define CHESS_ENGINE_MOVEGENERATOR_H

#include "Board.h"
#include "Move.h"
#include <vector>

class MoveGenerator {
public:
    std::vector<Move> generatePseudoLegalMoves(const Board& board) const;
    std::vector<Move> generateLegalMoves(Board& board) const;

    static bool isSquareAttacked(const Board& board, int row, int col, Color attackingColor);
    static bool isInCheck(const Board& board, Color color);

private:
    void generatePawnMoves(const Board&, int, int, std::vector<Move>&) const;
    void generateKnightMoves(const Board&, int, int, std::vector<Move>&) const;
    void generateBishopMoves(const Board&, int, int, std::vector<Move>&) const;
    void generateRookMoves(const Board&, int, int, std::vector<Move>&) const;
    void generateQueenMoves(const Board&, int, int, std::vector<Move>&) const;
    void generateKingMoves(const Board&, int, int, std::vector<Move>&) const;
};

#endif
