#ifndef CHESS_ENGINE_CHESSENGINE_H
#define CHESS_ENGINE_CHESSENGINE_H

#include "Board.h"
#include "MoveGenerator.h"
#include "Evaluator.h"
#include "SearchEngine.h"
#include <string>
#include <vector>
// chess engine contains searchengine , board over which search would be performed, movegenerator to generate all legal moves 
class ChessEngine {
private:
    Board board;
    MoveGenerator moveGenerator;
    SearchEngine searchEngine;

public:
    ChessEngine() = default;
    
    void newGame();
    bool loadFEN(const std::string& fen);

    Board& getBoard() { return board; }
    const Board& getBoard() const { return board; }

    std::vector<Move> legalMoves();
    bool makeMove(const Move& move);
    bool makeUCIMove(const std::string& uci);
    bool undoMove();

    Move getBestMove(int depth = 4, int timeMs = 0);
    void printBoard() const;
    std::string getFEN() const;

    static std::string movesToString(const std::vector<Move>& moves);
};

#endif
