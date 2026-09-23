#ifndef CHESS_ENGINE_BOARDSTATE_H
#define CHESS_ENGINE_BOARDSTATE_H

#include "Piece.h"
#include "Move.h"
/*
Boardstate is used to represent the current state of the board whose side it is to move , castling rights , enpassant etc
*/

struct BoardState {
    Color sideToMove = Color::WHITE;
    bool whiteKingSideCastle = true;
    bool whiteQueenSideCastle = true;
    bool blackKingSideCastle = true;
    bool blackQueenSideCastle = true;
    int enPassantRow = -1;
    int enPassantCol = -1;
    int halfmoveClock = 0;
    int fullmoveNumber = 1;
};
//  move state contain move pieces that has been captured in that move and also the the previous state of the board before that move 
struct MoveState {
    Move move;
    Piece capturedPiece;
    BoardState previousState;
};

#endif
