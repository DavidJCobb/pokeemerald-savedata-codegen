#include "lu/generated/struct-serialize//serialize_PokeNews.h"

#include "global.tv.h" // struct definition

void lu_BitstreamRead_PokeNews(struct lu_BitstreamState* state, const struct PokeNews* src) {
   src.kind = lu_BitstreamRead_u8(state, 8);
   src.state = lu_BitstreamRead_u8(state, 8);
   src.dayCountdown = lu_BitstreamRead_u16(state, 16);
}
void lu_BitstreamWrite_PokeNews(struct lu_BitstreamState* state, const struct PokeNews* src) {
   lu_BitstreamWrite_u8(state, src.kind, 8);
   lu_BitstreamWrite_u8(state, src.state, 8);
   lu_BitstreamWrite_u16(state, src.dayCountdown, 16);
}