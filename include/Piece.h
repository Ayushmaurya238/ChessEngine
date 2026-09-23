#ifndef CHESS_ENGINE_PIECE_H
#define CHESS_ENGINE_PIECE_H
//  enum class for piecetype and color 
enum class PieceType { NONE, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING };
enum class Color { WHITE, BLACK, NONE };

struct Piece {
    PieceType type = PieceType::NONE;
    Color color = Color::NONE;
};
//  method to get the opposite color 
inline Color opposite(Color c) {
    return c == Color::WHITE ? Color::BLACK :
           c == Color::BLACK ? Color::WHITE : Color::NONE;
}
//  getting the index of the color black 1 white 0 
inline int colorIndex(Color c) { return c == Color::BLACK ? 1 : 0; }

#endif
