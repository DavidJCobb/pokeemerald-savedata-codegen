#include "struct-serialize//serialize_Berry2.h"

#include "global.berry.h" // struct definition

#include "string_util.h" // gflib; for StringLength

// check constants:
#if BERRY_NAME_LENGTH != 6
   #error Constant `BERRY_NAME_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_Berry2(struct lu_BitstreamState* state, const struct Berry2* src) {
   lu_BitstreamRead_string(state, src.name, BERRY_NAME_LENGTH, 3);
   src.firmness = lu_BitstreamRead_u8(state, 8);
   src.size = lu_BitstreamRead_u16(state, 16);
   src.maxYield = lu_BitstreamRead_u8(state, 8);
   src.minYield = lu_BitstreamRead_u8(state, 8);
   description1 = (u8*) lu_BitstreamREad_u32(state, 32);
   description2 = (u8*) lu_BitstreamREad_u32(state, 32);
   src.stageDuration = lu_BitstreamRead_u8(state, 8);
   src.spicy = lu_BitstreamRead_u8(state, 8);
   src.dry = lu_BitstreamRead_u8(state, 8);
   src.sweet = lu_BitstreamRead_u8(state, 8);
   src.bitter = lu_BitstreamRead_u8(state, 8);
   src.sour = lu_BitstreamRead_u8(state, 8);
   src.smoothness = lu_BitstreamRead_u8(state, 8);
}
void lu_BitstreamWrite_Berry2(struct lu_BitstreamState* state, const struct Berry2* src) {
   lu_BitstreamWrite_string(state, src.name, BERRY_NAME_LENGTH, 3);
   lu_BitstreamWrite_u8(state, src.firmness, 8);
   lu_BitstreamWrite_u16(state, src.size, 16);
   lu_BitstreamWrite_u8(state, src.maxYield, 8);
   lu_BitstreamWrite_u8(state, src.minYield, 8);
   lu_BitstreamWrite_u32(state, src.description1, 32);
   lu_BitstreamWrite_u32(state, src.description2, 32);
   lu_BitstreamWrite_u8(state, src.stageDuration, 8);
   lu_BitstreamWrite_u8(state, src.spicy, 8);
   lu_BitstreamWrite_u8(state, src.dry, 8);
   lu_BitstreamWrite_u8(state, src.sweet, 8);
   lu_BitstreamWrite_u8(state, src.bitter, 8);
   lu_BitstreamWrite_u8(state, src.sour, 8);
   lu_BitstreamWrite_u8(state, src.smoothness, 8);
}