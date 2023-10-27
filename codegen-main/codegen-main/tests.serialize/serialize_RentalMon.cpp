#include "./tests.serialize//serialize_RentalMon.h"

#include "global.h"

// TODO:
// void lu_BitstreamRead_RentalMon(struct lu_BitstreamState* state, struct RentalMon* dst);

void lu_BitstreamWrite_RentalMon(struct lu_BitstreamState* state, const struct RentalMon* src) {
   lu_BitstreamWrite_u16(state, src.monId, 11);
   lu_BitstreamWrite_u32(state, src.personality, 32);
   lu_BitstreamWrite_u8(state, src.ivs, 8);
   lu_BitstreamWrite_u8(state, src.abilityNum, 8);
}