#include "Board.h"
#include "FENParser.h"
#include "MoveGenerator.h"
#include <cassert>
#include <cstdint>
#include <iostream>

uint64_t perft(Board& b, MoveGenerator& mg, int depth) {
    if (depth == 0) return 1;
    uint64_t nodes = 0;
    for (const auto& m : mg.generateLegalMoves(b)) {
        b.makeMove(m);
        nodes += perft(b, mg, depth - 1);
        b.undoMove();
    }
    return nodes;
}

void check(const char* name, const char* fen, int depth, uint64_t expected) {
    Board b = FENParser::fromFEN(fen);
    MoveGenerator mg;
    uint64_t got = perft(b, mg, depth);
    std::cout << name << " depth " << depth << ": " << got << "\n";
    assert(got == expected);
}

int main() {
    const char* start = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    check("Start", start, 1, 20);
    check("Start", start, 2, 400);
    check("Start", start, 3, 8902);
    check("Start", start, 4, 197281);

    const char* kiwipete = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    check("Kiwipete", kiwipete, 1, 48);
    check("Kiwipete", kiwipete, 2, 2039);

    const char* pos3 = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1";
    check("Position 3", pos3, 1, 14);
    check("Position 3", pos3, 2, 191);

    std::cout << "All perft tests passed.\n";
}
