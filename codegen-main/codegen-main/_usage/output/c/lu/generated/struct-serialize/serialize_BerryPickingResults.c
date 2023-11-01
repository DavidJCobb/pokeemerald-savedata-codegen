#include "lu/generated/struct-serialize//serialize_BerryPickingResults.h"

#include "global.h" // struct definition

void lu_BitstreamRead_BerryPickingResults(struct lu_BitstreamState* state, const struct BerryPickingResults* src) {
   src.bestScore = lu_BitstreamRead_u32(state, 32);
   src.berriesPicked = lu_BitstreamRead_u16(state, 16);
   src.berriesPickedInRow = lu_BitstreamRead_u16(state, 16);
   src.field_8 = lu_BitstreamRead_u8(state, 8);
   src.field_9 = lu_BitstreamRead_u8(state, 8);
   src.field_A = lu_BitstreamRead_u8(state, 8);
   src.field_B = lu_BitstreamRead_u8(state, 8);
   src.field_C = lu_BitstreamRead_u8(state, 8);
   src.field_D = lu_BitstreamRead_u8(state, 8);
   src.field_E = lu_BitstreamRead_u8(state, 8);
   src.field_F = lu_BitstreamRead_u8(state, 8);
}
void lu_BitstreamWrite_BerryPickingResults(struct lu_BitstreamState* state, const struct BerryPickingResults* src) {
   lu_BitstreamWrite_u32(state, src.bestScore, 32);
   lu_BitstreamWrite_u16(state, src.berriesPicked, 16);
   lu_BitstreamWrite_u16(state, src.berriesPickedInRow, 16);
   lu_BitstreamWrite_u8(state, src.field_8, 8);
   lu_BitstreamWrite_u8(state, src.field_9, 8);
   lu_BitstreamWrite_u8(state, src.field_A, 8);
   lu_BitstreamWrite_u8(state, src.field_B, 8);
   lu_BitstreamWrite_u8(state, src.field_C, 8);
   lu_BitstreamWrite_u8(state, src.field_D, 8);
   lu_BitstreamWrite_u8(state, src.field_E, 8);
   lu_BitstreamWrite_u8(state, src.field_F, 8);
}