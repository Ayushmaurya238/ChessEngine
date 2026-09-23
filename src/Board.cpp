#include "Board.h"
#include "Move.h"
#include <iostream>
#include <cctype>
// create a mapping function that would directly map pieces with a character for better representation for differentiating bw white and black lower and upper are used as characters
static char pieceChar(Piece p)
{
    if (p.type == PieceType::NONE)
        return '.';
    char c = '.';
    switch (p.type)
    {
    case PieceType::PAWN:
        c = 'P';
        break;
    case PieceType::KNIGHT:
        c = 'N';
        break;
    case PieceType::BISHOP:
        c = 'B';
        break;
    case PieceType::ROOK:
        c = 'R';
        break;
    case PieceType::QUEEN:
        c = 'Q';
        break;
    case PieceType::KING:
        c = 'K';
        break;
    default:
        break;
    }
    if (p.color == Color::BLACK)
        c = char(std::tolower((unsigned char)c));
    return c;
}
// constructor to initialize the board with initial positions
Board::Board() { initialize(); }
//  clearing the board to starting by
void Board::clear()
{
    for (auto &row : squares)
        for (auto &p : row)
            p = Piece{};
    state = BoardState{};
    history.clear();
}
// initialise the board using
void Board::initialize()
{
    clear();
    const PieceType back[8] = {
        PieceType::ROOK, PieceType::KNIGHT, PieceType::BISHOP, PieceType::QUEEN,
        PieceType::KING, PieceType::BISHOP, PieceType::KNIGHT, PieceType::ROOK};
    for (int c = 0; c < 8; c++)
    {
        squares[0][c] = {back[c], Color::BLACK};
        squares[1][c] = {PieceType::PAWN, Color::BLACK};
        squares[6][c] = {PieceType::PAWN, Color::WHITE};
        squares[7][c] = {back[c], Color::WHITE};
    }
    state = BoardState{};
}

Piece Board::getPiece(int row, int col) const
{
    //  check
    if (row < 0 || row >= 8 || col < 0 || col >= 8)
        return Piece{};
    return squares[row][col];
}

void Board::setPiece(int row, int col, Piece piece)
{
    if (row >= 0 && row < 8 && col >= 0 && col < 8)
        squares[row][col] = piece;
}

bool Board::canCastleKingSide(Color c) const
{
    return c == Color::WHITE ? state.whiteKingSideCastle : state.blackKingSideCastle;
}
bool Board::canCastleQueenSide(Color c) const
{
    return c == Color::WHITE ? state.whiteQueenSideCastle : state.blackQueenSideCastle;
}
void Board::setCastlingRights(Color c, bool kingSide, bool queenSide)
{
    if (c == Color::WHITE)
    {
        state.whiteKingSideCastle = kingSide;
        state.whiteQueenSideCastle = queenSide;
    }
    else
    {
        state.blackKingSideCastle = kingSide;
        state.blackQueenSideCastle = queenSide;
    }
}
// to find the kings position 
int Board::findKing(Color c) const
{
    for (int r = 0; r < 8; r++)
        for (int col = 0; col < 8; col++)
            if (squares[r][col].type == PieceType::KING && squares[r][col].color == c)
                return r * 8 + col;
    return -1;
}
//  make a move 
void Board::makeMove(const Move &move)
{
    Piece moving = squares[move.fromRow][move.fromCol];
    Piece captured = squares[move.toRow][move.toCol];

    MoveState ms{move, captured, state};
    history.push_back(ms);

    if (move.isEnPassant)
    {
        int capRow = moving.color == Color::WHITE ? move.toRow + 1 : move.toRow - 1;
        captured = squares[capRow][move.toCol];
        history.back().capturedPiece = captured;
        squares[capRow][move.toCol] = Piece{};
    }

    squares[move.fromRow][move.fromCol] = Piece{};

    if (move.isCastling)
    {
        squares[move.toRow][move.toCol] = moving;
        if (move.toCol == 6)
        {
            squares[move.toRow][5] = squares[move.toRow][7];
            squares[move.toRow][7] = Piece{};
        }
        else if (move.toCol == 2)
        {
            squares[move.toRow][3] = squares[move.toRow][0];
            squares[move.toRow][0] = Piece{};
        }
    }
    else
    {
        Piece placed = moving;
        if (move.promotionPiece != PieceType::NONE)
            placed.type = move.promotionPiece;
        squares[move.toRow][move.toCol] = placed;
    }

    // Castling rights disappear when a king or rook moves.
    if (moving.type == PieceType::KING)
    {
        setCastlingRights(moving.color, false, false);
    }
    else if (moving.type == PieceType::ROOK)
    {
        if (moving.color == Color::WHITE && move.fromRow == 7)
        {
            if (move.fromCol == 0)
                state.whiteQueenSideCastle = false;
            if (move.fromCol == 7)
                state.whiteKingSideCastle = false;
        }
        if (moving.color == Color::BLACK && move.fromRow == 0)
        {
            if (move.fromCol == 0)
                state.blackQueenSideCastle = false;
            if (move.fromCol == 7)
                state.blackKingSideCastle = false;
        }
    }

    // Capturing a rook on its original square removes that castling right.
    if (captured.type == PieceType::ROOK)
    {
        if (captured.color == Color::WHITE && move.toRow == 7)
        {
            if (move.toCol == 0)
                state.whiteQueenSideCastle = false;
            if (move.toCol == 7)
                state.whiteKingSideCastle = false;
        }
        if (captured.color == Color::BLACK && move.toRow == 0)
        {
            if (move.toCol == 0)
                state.blackQueenSideCastle = false;
            if (move.toCol == 7)
                state.blackKingSideCastle = false;
        }
    }

    state.enPassantRow = state.enPassantCol = -1;
    if (moving.type == PieceType::PAWN && std::abs(move.toRow - move.fromRow) == 2)
    {
        state.enPassantRow = (move.fromRow + move.toRow) / 2;
        state.enPassantCol = move.fromCol;
    }

    if (moving.type == PieceType::PAWN || captured.type != PieceType::NONE)
        state.halfmoveClock = 0;
    else
        state.halfmoveClock++;

    if (moving.color == Color::BLACK)
        state.fullmoveNumber++;
    state.sideToMove = opposite(state.sideToMove);
}

void Board::undoMove()
{
    if (history.empty())
        return;
    MoveState ms = history.back();
    history.pop_back();

    const Move &move = ms.move;
    BoardState old = ms.previousState;

    Piece moving = squares[move.toRow][move.toCol];
    if (move.promotionPiece != PieceType::NONE)
        moving.type = PieceType::PAWN;

    if (move.isCastling)
    {
        squares[move.toRow][move.toCol] = Piece{};
        if (move.toCol == 6)
        {
            squares[move.toRow][7] = squares[move.toRow][5];
            squares[move.toRow][5] = Piece{};
        }
        else if (move.toCol == 2)
        {
            squares[move.toRow][0] = squares[move.toRow][3];
            squares[move.toRow][3] = Piece{};
        }
        squares[move.fromRow][move.fromCol] = moving;
    }
    else
    {
        squares[move.toRow][move.toCol] = Piece{};
        squares[move.fromRow][move.fromCol] = moving;
        if (move.isEnPassant)
        {
            int capRow = moving.color == Color::WHITE ? move.toRow + 1 : move.toRow - 1;
            squares[capRow][move.toCol] = ms.capturedPiece;
        }
        else
        {
            squares[move.toRow][move.toCol] = ms.capturedPiece;
        }
    }
    state = old;
}

void Board::print() const
{
    std::cout << "\n    a b c d e f g h\n";
    std::cout << "  +-----------------+\n";
    for (int r = 0; r < 8; r++)
    {
        std::cout << 8 - r << " | ";
        for (int c = 0; c < 8; c++)
            std::cout << pieceChar(squares[r][c]) << ' ';
        std::cout << "| " << 8 - r << '\n';
    }
    std::cout << "  +-----------------+\n";
    std::cout << "    a b c d e f g h\n";
    std::cout << "Side: " << (state.sideToMove == Color::WHITE ? "White" : "Black") << '\n';
}

std::string Board::toFEN() const
{
    std::string fen;
    for (int r = 0; r < 8; r++)
    {
        int empty = 0;
        for (int c = 0; c < 8; c++)
        {
            char ch = pieceChar(squares[r][c]);
            if (ch == '.')
                empty++;
            else
            {
                if (empty)
                {
                    fen += char('0' + empty);
                    empty = 0;
                }
                fen += ch;
            }
        }
        if (empty)
            fen += char('0' + empty);
        if (r != 7)
            fen += '/';
    }
    fen += state.sideToMove == Color::WHITE ? " w " : " b ";
    std::string cast;
    if (state.whiteKingSideCastle)
        cast += 'K';
    if (state.whiteQueenSideCastle)
        cast += 'Q';
    if (state.blackKingSideCastle)
        cast += 'k';
    if (state.blackQueenSideCastle)
        cast += 'q';
    fen += cast.empty() ? "-" : cast;
    fen += ' ';
    if (state.enPassantRow == -1)
        fen += '-';
    else
        fen += squareToString(state.enPassantRow, state.enPassantCol);
    fen += ' ' + std::to_string(state.halfmoveClock);
    fen += ' ' + std::to_string(state.fullmoveNumber);
    return fen;
}
