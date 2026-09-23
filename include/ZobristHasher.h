#ifndef CHESS_ENGINE_ZOBRISTHASHER_H
#define CHESS_ENGINE_ZOBRISTHASHER_H

#include "Board.h"
#include <cstdint>

class ZobristHasher {
private:
    uint64_t pieceKeys[2][6][64]{};
    uint64_t sideKey{};
    uint64_t castleKeys[16]{};
    uint64_t epKeys[8]{};
    bool initialized = false;

    void init();

public:
    ZobristHasher();
    uint64_t hash(const Board& board);
};

#endif
