#include "struct-serialize//serialize_BerryTree.h"

#include "global.berry.h" // struct definition

void lu_BitstreamRead_BerryTree(struct lu_BitstreamState* state, const struct BerryTree* src) {
   src.berry = lu_BitstreamRead_u8(state, 8);
   src.stage = lu_BitstreamRead_u8(state, 7);
   src.stopGrowth = lu_BitstreamRead_bool(state, src.stopGrowth);
   src.minutesUntilNextStage = lu_BitstreamRead_u16(state, 16);
   src.berryYield = lu_BitstreamRead_u8(state, 8);
   src.regrowthCount = lu_BitstreamRead_u8(state, 4);
   src.watered1 = lu_BitstreamRead_bool(state, src.watered1);
   src.watered2 = lu_BitstreamRead_bool(state, src.watered2);
   src.watered3 = lu_BitstreamRead_bool(state, src.watered3);
   src.watered4 = lu_BitstreamRead_bool(state, src.watered4);
}
void lu_BitstreamWrite_BerryTree(struct lu_BitstreamState* state, const struct BerryTree* src) {
   lu_BitstreamWrite_u8(state, src.berry, 8);
   lu_BitstreamWrite_u8(state, src.stage, 7);
   lu_BitstreamWrite_bool(state, src.stopGrowth);
   lu_BitstreamWrite_u16(state, src.minutesUntilNextStage, 16);
   lu_BitstreamWrite_u8(state, src.berryYield, 8);
   lu_BitstreamWrite_u8(state, src.regrowthCount, 4);
   lu_BitstreamWrite_bool(state, src.watered1);
   lu_BitstreamWrite_bool(state, src.watered2);
   lu_BitstreamWrite_bool(state, src.watered3);
   lu_BitstreamWrite_bool(state, src.watered4);
}