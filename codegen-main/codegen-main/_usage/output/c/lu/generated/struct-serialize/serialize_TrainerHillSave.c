#include "lu/generated/struct-serialize//serialize_TrainerHillSave.h"

#include "global.h" // struct definition

void lu_BitstreamRead_TrainerHillSave(struct lu_BitstreamState* state, const struct TrainerHillSave* src) {
   src.timer = lu_BitstreamRead_u32(state, 32);
   src.bestTime = lu_BitstreamRead_u32(state, 32);
   src.unk_3D6C = lu_BitstreamRead_u8(state, 8);
   src.unused = lu_BitstreamRead_u8(state, 8);
   src.receivedPrize = lu_BitstreamRead_bool(state, src.receivedPrize);
   src.checkedFinalTime = lu_BitstreamRead_bool(state, src.checkedFinalTime);
   src.spokeToOwner = lu_BitstreamRead_bool(state, src.spokeToOwner);
   src.hasLost = lu_BitstreamRead_bool(state, src.hasLost);
   src.maybeECardScanDuringChallenge = lu_BitstreamRead_bool(state, src.maybeECardScanDuringChallenge);
   src.field_3D6E_0f = lu_BitstreamRead_bool(state, src.field_3D6E_0f);
   src.mode = lu_BitstreamRead_u8(state, 2);
}
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