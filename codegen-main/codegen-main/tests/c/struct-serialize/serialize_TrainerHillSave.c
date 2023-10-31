#include "struct-serialize//serialize_TrainerHillSave.h"

#include "global.h" // struct definition

// TODO:
// void lu_BitstreamRead_TrainerHillSave(struct lu_BitstreamState* state, struct TrainerHillSave* dst);

void lu_BitstreamWrite_TrainerHillSave(struct lu_BitstreamState* state, const struct TrainerHillSave* src) {
   lu_BitstreamWrite_u32(state, src.timer, 32);
   lu_BitstreamWrite_u32(state, src.bestTime, 32);
   lu_BitstreamWrite_u8(state, src.unk_3D6C, 8);
   lu_BitstreamWrite_u8(state, src.unused, 8);
   lu_BitstreamWrite_bool(state, src.receivedPrize);
   lu_BitstreamWrite_bool(state, src.checkedFinalTime);
   lu_BitstreamWrite_bool(state, src.spokeToOwner);
   lu_BitstreamWrite_bool(state, src.hasLost);
   lu_BitstreamWrite_bool(state, src.maybeECardScanDuringChallenge);
   lu_BitstreamWrite_bool(state, src.field_3D6E_0f);
   lu_BitstreamWrite_u8(state, src.mode, 2);
}