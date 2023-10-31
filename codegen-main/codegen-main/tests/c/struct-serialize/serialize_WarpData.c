#include "struct-serialize//serialize_WarpData.h"

#include "global.h" // struct definition

// TODO:
// void lu_BitstreamRead_WarpData(struct lu_BitstreamState* state, struct WarpData* dst);

void lu_BitstreamWrite_WarpData(struct lu_BitstreamState* state, const struct WarpData* src) {
   lu_BitstreamWrite_u8(state, src.mapGroup, 8);
   lu_BitstreamWrite_u8(state, src.mapNum, 8);
   lu_BitstreamWrite_u8(state, src.warpId, 8);
   lu_BitstreamWrite_u16(state, src.x, 16);
   lu_BitstreamWrite_u16(state, src.y, 16);
}