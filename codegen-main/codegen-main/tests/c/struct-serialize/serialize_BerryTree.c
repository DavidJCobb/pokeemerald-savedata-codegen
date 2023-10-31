#include "struct-serialize//serialize_BerryTree.h"

#include "global.berry.h" // struct definition

// TODO:
// void lu_BitstreamRead_BerryTree(struct lu_BitstreamState* state, struct BerryTree* dst);

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