#include "lu/generated/struct-serialize//serialize_RentalMon.h"

#include "global.h" // struct definition

void lu_BitstreamRead_RentalMon(struct lu_BitstreamState* state, const struct RentalMon* src) {
   src.monId = lu_BitstreamRead_u16(state, 11);
   src.personality = lu_BitstreamRead_u32(state, 32);
   src.ivs = lu_BitstreamRead_u8(state, 8);
   src.abilityNum = lu_BitstreamRead_u8(state, 8);
}
void lu_BitstreamWrite_RentalMon(struct lu_BitstreamState* state, const struct RentalMon* src) {
   lu_BitstreamWrite_u16(state, src.monId, 11);
   lu_BitstreamWrite_u32(state, src.personality, 32);
   lu_BitstreamWrite_u8(state, src.ivs, 8);
   lu_BitstreamWrite_u8(state, src.abilityNum, 8);
}