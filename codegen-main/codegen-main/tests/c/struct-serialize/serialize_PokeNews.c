#include "struct-serialize//serialize_PokeNews.h"

#include "global.tv.h" // struct definition

// TODO:
// void lu_BitstreamRead_PokeNews(struct lu_BitstreamState* state, struct PokeNews* dst);

void lu_BitstreamWrite_PokeNews(struct lu_BitstreamState* state, const struct PokeNews* src) {
   lu_BitstreamWrite_u8(state, src.kind, 8);
   lu_BitstreamWrite_u8(state, src.state, 8);
   lu_BitstreamWrite_u16(state, src.dayCountdown, 16);
}