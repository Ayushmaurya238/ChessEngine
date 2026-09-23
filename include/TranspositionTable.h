#ifndef CHESS_ENGINE_TRANSPOSITIONTABLE_H
#define CHESS_ENGINE_TRANSPOSITIONTABLE_H

#include "Move.h"
#include <cstdint>
#include <unordered_map>

enum class TTFlag { EXACT, LOWERBOUND, UPPERBOUND };

struct TTEntry {
    uint64_t hash = 0;
    int depth = -1;
    int score = 0;
    TTFlag flag = TTFlag::EXACT;
    Move bestMove;
};

class TranspositionTable {
private:
    std::unordered_map<uint64_t, TTEntry> table;
    size_t maxEntries;

public:
    explicit TranspositionTable(size_t maxEntries = 500000) : maxEntries(maxEntries) {}

    void clear() { table.clear(); }
    bool probe(uint64_t hash, TTEntry& entry) const;
    void store(const TTEntry& entry);
    size_t size() const { return table.size(); }
};

#endif
