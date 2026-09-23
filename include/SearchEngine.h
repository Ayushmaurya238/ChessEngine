#ifndef CHESS_ENGINE_SEARCHENGINE_H
#define CHESS_ENGINE_SEARCHENGINE_H

#include "Board.h"
#include "MoveGenerator.h"
#include "Evaluator.h"
#include "ZobristHasher.h"
#include "TranspositionTable.h"
#include <atomic>
#include <chrono>
#include <cstdint>
#include <vector>

class SearchEngine {
public:
    struct SearchInfo {
        int depth = 0;
        int score = 0;
        uint64_t nodes = 0;
        double seconds = 0.0;
        uint64_t ttHits = 0;
    };

private:
    MoveGenerator moveGenerator;
    Evaluator evaluator;
    ZobristHasher hasher;
    TranspositionTable tt;
    std::atomic<bool> stopFlag{false};
    uint64_t nodes = 0;
    uint64_t ttHits = 0;
    std::chrono::steady_clock::time_point deadline{};
    bool useDeadline = false;
    Move rootBestMove;
    SearchInfo info;

    static constexpr int INF = 100000000;
    static constexpr int MATE = 1000000;

    int alphaBeta(Board& board, int depth, int alpha, int beta, int ply);
    int quiescence(Board& board, int alpha, int beta, int ply);
    std::vector<Move> orderMoves(Board& board, std::vector<Move> moves,
                                 const Move* ttMove = nullptr) const;
    int moveScore(Board& board, const Move& move) const;
    bool shouldStop() const;
    void searchRoot(Board& board, int depth, Move& best, int& bestScore);

public:
    SearchEngine(size_t ttSize = 500000) : tt(ttSize) {}

    Move findBestMove(Board& board, int maxDepth = 4, int timeMs = 0);
    void stop() { stopFlag.store(true); }
    void clear();
    const SearchInfo& getInfo() const { return info; }
};

#endif
