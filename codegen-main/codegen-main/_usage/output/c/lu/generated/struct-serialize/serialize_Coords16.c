#include "lu/generated/struct-serialize//serialize_Coords16.h"

#include "global.h" // struct definition

void lu_BitstreamRead_Coords16(struct lu_BitstreamState* state, const struct Coords16* src) {
   src.x = lu_BitstreamRead_u16(state, 16);
   src.y = lu_BitstreamRead_u16(state, 16);
}
void lu_BitstreamWrite_Coords16(struct lu_BitstreamState* state, const struct Coords16* src) {
   lu_BitstreamWrite_u16(state, src.x, 16);
   lu_BitstreamWrite_u16(state, src.y, 16);
}