#include "struct-serialize//serialize_Coords16.h"

#include "global.h" // struct definition

// TODO:
// void lu_BitstreamRead_Coords16(struct lu_BitstreamState* state, struct Coords16* dst);

void lu_BitstreamWrite_Coords16(struct lu_BitstreamState* state, const struct Coords16* src) {
   lu_BitstreamWrite_u16(state, src.x, 16);
   lu_BitstreamWrite_u16(state, src.y, 16);
}