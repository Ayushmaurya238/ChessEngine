#ifndef CHESS_ENGINE_EVALUATOR_H
#define CHESS_ENGINE_EVALUATOR_H

#include "Board.h"

class Evaluator {
public:
    int evaluate(const Board& board) const;

private:
    int material(const Board& board) const;
    int pieceSquare(const Board& board) const;
    int mobility(Board& board) const;
};

#endif
