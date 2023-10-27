#include "./tests.serialize//serialize_BerryPickingResults.h"

#include "global.h" // struct definition

// TODO:
// void lu_BitstreamRead_BerryPickingResults(struct lu_BitstreamState* state, struct BerryPickingResults* dst);

void lu_BitstreamWrite_BerryPickingResults(struct lu_BitstreamState* state, const struct BerryPickingResults* src) {
   lu_BitstreamWrite_u32(state, src.bestScore, 32);
   lu_BitstreamWrite_u32(state, src.berriesPicked, 32);
   lu_BitstreamWrite_u32(state, src.berriesPickedInRow, 32);
   lu_BitstreamWrite_u8(state, src.field_8, 8);
   lu_BitstreamWrite_u8(state, src.field_9, 8);
   lu_BitstreamWrite_u8(state, src.field_A, 8);
   lu_BitstreamWrite_u8(state, src.field_B, 8);
   lu_BitstreamWrite_u8(state, src.field_C, 8);
   lu_BitstreamWrite_u8(state, src.field_D, 8);
   lu_BitstreamWrite_u8(state, src.field_E, 8);
   lu_BitstreamWrite_u8(state, src.field_F, 8);
}