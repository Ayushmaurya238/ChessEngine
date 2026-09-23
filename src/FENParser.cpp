#include "FENParser.h"
#include <sstream>
#include <stdexcept>
#include <cctype>

static Piece fromChar(char c){
    Color color=std::isupper((unsigned char)c)?Color::WHITE:Color::BLACK;
    char x=(char)std::tolower((unsigned char)c);PieceType t=PieceType::NONE;
    switch(x){case 'p':t=PieceType::PAWN;break;case'n':t=PieceType::KNIGHT;break;case'b':t=PieceType::BISHOP;break;case'r':t=PieceType::ROOK;break;case'q':t=PieceType::QUEEN;break;case'k':t=PieceType::KING;break;default:throw std::runtime_error("Invalid FEN piece");}
    return {t,color};
}
Board FENParser::fromFEN(const std::string& fen){
    std::istringstream ss(fen);std::string placement,side,castle,ep;int half=0,full=1;
    if(!(ss>>placement>>side>>castle>>ep>>half>>full))throw std::runtime_error("Invalid FEN: expected 6 fields");
    Board b;b.clear();int r=0,c=0;
    for(char ch:placement){
        if(ch=='/'){if(c!=8)throw std::runtime_error("Invalid FEN rank");r++;c=0;continue;}
        if(r>=8)throw std::runtime_error("Invalid FEN board");
        if(std::isdigit((unsigned char)ch))c+=ch-'0';
        else {if(c>=8)throw std::runtime_error("Invalid FEN square");b.setPiece(r,c++,fromChar(ch));}
    }
    if(r!=7||c!=8)throw std::runtime_error("Invalid FEN board");
    BoardState st;st.sideToMove=side=="w"?Color::WHITE:Color::BLACK;
    st.whiteKingSideCastle=castle.find('K')!=std::string::npos;st.whiteQueenSideCastle=castle.find('Q')!=std::string::npos;
    st.blackKingSideCastle=castle.find('k')!=std::string::npos;st.blackQueenSideCastle=castle.find('q')!=std::string::npos;
    st.halfmoveClock=half;st.fullmoveNumber=full;
    if(ep!="-"){if(ep.size()!=2)throw std::runtime_error("Invalid en passant");st.enPassantCol=ep[0]-'a';st.enPassantRow=8-(ep[1]-'0');}
    b.setState(st);return b;
}
std::string FENParser::toFEN(const Board& b){return b.toFEN();}
