#ifndef CHESS_ENGINE_BOARD_H
#define CHESS_ENGINE_BOARD_H

#include "Piece.h"
#include "BoardState.h"
#include <string>
#include <vector>

class Board {
private:
    Piece squares[8][8]{};
    BoardState state;
    std::vector<MoveState> history;

public:
    Board();

    void initialize();
    void clear();

    Piece getPiece(int row, int col) const; // to get the piece at row , col
    void setPiece(int row, int col, Piece piece);// to set a piece at row , col

    Color getSideToMove() const { return state.sideToMove; }// to get which side turn it is 
    const BoardState& getState() const { return state; }//get the current state of the board 
    void setState(const BoardState& s) { state = s; }// sets the state of the board 
    void setSideToMove(Color c) { state.sideToMove = c; }//set side to move 

    bool canCastleKingSide(Color c) const;// see whether color c could do castle king side
    bool canCastleQueenSide(Color c) const;// see whether color c could castle queen side
    void setCastlingRights(Color c, bool kingSide, bool queenSide);// to  set the castling write 

    int getEnPassantRow() const { return state.enPassantRow; }// get enpassant row 
    int getEnPassantCol() const { return state.enPassantCol; }// get enpassant col

    void print() const;// print current state 
    std::string toFEN() const;

    void makeMove(const Move& move);// to do the move 
    void undoMove();// for undoing the last move 
    bool canUndo() const { return !history.empty(); }// if undo is possible like when the game starts then no undo is possible 

    int findKing(Color c) const;// get the position of the king 
};

#endif
