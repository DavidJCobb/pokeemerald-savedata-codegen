#include "lu/generated/struct-serialize//serialize_WarpData.h"

#include "global.h" // struct definition

void lu_BitstreamRead_WarpData(struct lu_BitstreamState* state, const struct WarpData* src) {
   src.mapGroup = lu_BitstreamRead_u8(state, 8);
   src.mapNum = lu_BitstreamRead_u8(state, 8);
   src.warpId = lu_BitstreamRead_u8(state, 8);
   src.x = lu_BitstreamRead_u16(state, 16);
   src.y = lu_BitstreamRead_u16(state, 16);
}
void lu_BitstreamWrite_WarpData(struct lu_BitstreamState* state, const struct WarpData* src) {
   lu_BitstreamWrite_u8(state, src.mapGroup, 8);
   lu_BitstreamWrite_u8(state, src.mapNum, 8);
   lu_BitstreamWrite_u8(state, src.warpId, 8);
   lu_BitstreamWrite_u16(state, src.x, 16);
   lu_BitstreamWrite_u16(state, src.y, 16);
}