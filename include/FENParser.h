#ifndef CHESS_ENGINE_FENPARSER_H
#define CHESS_ENGINE_FENPARSER_H

#include "Board.h"
#include <string>

class FENParser {
public:
    static Board fromFEN(const std::string& fen);
    static std::string toFEN(const Board& board);
};

#endif
