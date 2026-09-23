#include "TranspositionTable.h"

bool TranspositionTable::probe(uint64_t h, TTEntry& e) const {
    auto it=table.find(h); if(it==table.end())return false; e=it->second; return true;
}
void TranspositionTable::store(const TTEntry& e){
    if(table.size()>=maxEntries){
        table.erase(table.begin());
    }
    auto it=table.find(e.hash);
    if(it==table.end()||e.depth>=it->second.depth)table[e.hash]=e;
}
