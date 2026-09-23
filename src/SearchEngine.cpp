#include "SearchEngine.h"
#include "Move.h"
#include <algorithm>
#include <limits>

bool SearchEngine::shouldStop() const {
    if(stopFlag.load()) return true;
    if(useDeadline && std::chrono::steady_clock::now()>=deadline) return true;
    return false;
}
void SearchEngine::clear(){tt.clear();nodes=ttHits=0;stopFlag.store(false);}

int SearchEngine::moveScore(Board& b,const Move& m) const{
    Piece moving=b.getPiece(m.fromRow,m.fromCol), captured=b.getPiece(m.toRow,m.toCol);
    auto pv=[](PieceType p){switch(p){case PieceType::PAWN:return 100;case PieceType::KNIGHT:return 320;case PieceType::BISHOP:return 330;case PieceType::ROOK:return 500;case PieceType::QUEEN:return 900;default:return 0;}};
    int score=0;
    if(m.isCapture) score+=100000+10*pv(captured.type)-pv(moving.type);
    if(m.isEnPassant) score+=100000+10*100-pv(moving.type);
    if(m.promotionPiece!=PieceType::NONE) score+=9000+pv(m.promotionPiece);
    if(m.isCastling)score+=50;
    return score;
}
std::vector<Move> SearchEngine::orderMoves(Board& b,std::vector<Move> moves,const Move* ttMove) const{
    std::sort(moves.begin(),moves.end(),[&](const Move&a,const Move&c){
        int sa=moveScore(b,a),sc=moveScore(b,c);
        if(ttMove&&a==*ttMove)sa+=10000000;
        if(ttMove&&c==*ttMove)sc+=10000000;
        return sa>sc;
    });return moves;
}

int SearchEngine::quiescence(Board& b,int alpha,int beta,int ply){
    if(shouldStop())return 0;
    nodes++;
    int stand=evaluator.evaluate(b);
    if(b.getSideToMove()==Color::BLACK)stand=-stand;
    if(stand>=beta)return beta;
    if(stand>alpha)alpha=stand;
    auto legal=moveGenerator.generateLegalMoves(b);
    std::vector<Move> tactical;
    for(auto&m:legal)if(m.isCapture||m.promotionPiece!=PieceType::NONE)tactical.push_back(m);
    tactical=orderMoves(b,tactical);
    for(auto&m:tactical){
        b.makeMove(m);
        int score=-quiescence(b,-beta,-alpha,ply+1);
        b.undoMove();
        if(shouldStop())return 0;
        if(score>=beta)return beta;
        if(score>alpha)alpha=score;
    }
    return alpha;
}

int SearchEngine::alphaBeta(Board& b,int depth,int alpha,int beta,int ply){
    if(shouldStop())return 0;
    nodes++;
    uint64_t h=hasher.hash(b);
    TTEntry ent; Move ttMove;
    if(tt.probe(h,ent)){
        ttHits++; ttMove=ent.bestMove;
        if(ent.depth>=depth){
            if(ent.flag==TTFlag::EXACT)return ent.score;
            if(ent.flag==TTFlag::LOWERBOUND)alpha=std::max(alpha,ent.score);
            else if(ent.flag==TTFlag::UPPERBOUND)beta=std::min(beta,ent.score);
            if(alpha>=beta)return ent.score;
        }
    }
    if(depth<=0)return quiescence(b,alpha,beta,ply);

    auto moves=moveGenerator.generateLegalMoves(b);
    if(moves.empty()){
        if(MoveGenerator::isInCheck(b,b.getSideToMove())) return -MATE+ply;
        return 0;
    }
    moves=orderMoves(b,moves,tt.probe(h,ent)?&ttMove:nullptr);
    int originalAlpha=alpha,best=-INF;Move bestMove=moves[0];
    for(auto&m:moves){
        b.makeMove(m);
        int score=-alphaBeta(b,depth-1,-beta,-alpha,ply+1);
        b.undoMove();
        if(shouldStop())return 0;
        if(score>best){best=score;bestMove=m;}
        alpha=std::max(alpha,score);
        if(alpha>=beta)break;
    }
    TTFlag flag=TTFlag::EXACT;
    if(best<=originalAlpha)flag=TTFlag::UPPERBOUND;
    else if(best>=beta)flag=TTFlag::LOWERBOUND;
    tt.store({h,depth,best,flag,bestMove});
    return best;
}

void SearchEngine::searchRoot(Board& b,int depth,Move& best,int& bestScore){
    auto moves=moveGenerator.generateLegalMoves(b);
    if(moves.empty()){best=Move();bestScore=0;return;}
    TTEntry ent;uint64_t h=hasher.hash(b);Move ttMove;Move* ptr=nullptr;
    if(tt.probe(h,ent)){ttMove=ent.bestMove;ptr=&ttMove;}
    moves=orderMoves(b,moves,ptr);
    int alpha=-INF,beta=INF;
    bestScore=-INF;best=moves[0];
    for(auto&m:moves){
        b.makeMove(m);int score=-alphaBeta(b,depth-1,-beta,-alpha,1);b.undoMove();
        if(shouldStop())return;
        if(score>bestScore){bestScore=score;best=m;}
        alpha=std::max(alpha,score);
    }
}

Move SearchEngine::findBestMove(Board& b,int maxDepth,int timeMs){
    stopFlag.store(false);nodes=ttHits=0;info={};useDeadline=timeMs>0;
    if(useDeadline)deadline=std::chrono::steady_clock::now()+std::chrono::milliseconds(timeMs);
    auto start=std::chrono::steady_clock::now();
    Move best;int bestScore=0;
    for(int d=1;d<=maxDepth;d++){
        Move candidate;int score=0;searchRoot(b,d,candidate,score);
        if(shouldStop())break;
        best=candidate;bestScore=score;info.depth=d;info.score=score;
    }
    info.nodes=nodes;info.ttHits=ttHits;info.seconds=std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();
    return best;
}
