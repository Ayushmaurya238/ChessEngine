#ifndef CHESS_ENGINE_UCI_H
#define CHESS_ENGINE_UCI_H

#include "ChessEngine.h"
#include <string>

class UCI {
private:
    ChessEngine engine;

    void handlePosition(const std::string& command);
    void handleGo(const std::string& command);

public:
    void run();
};

#endif
