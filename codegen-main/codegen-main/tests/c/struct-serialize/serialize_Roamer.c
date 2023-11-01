#include "struct-serialize//serialize_Roamer.h"

#include "global.h" // struct definition

void lu_BitstreamRead_Roamer(struct lu_BitstreamState* state, const struct Roamer* src) {
   src.ivs = lu_BitstreamRead_u32(state, 32);
   src.personality = lu_BitstreamRead_u32(state, 32);
   src.species = lu_BitstreamRead_u16(state, 11);
   src.hp = lu_BitstreamRead_u16(state, 16);
   src.level = lu_BitstreamRead_u8(state, 7);
   src.status = lu_BitstreamRead_u8(state, 8);
   src.cool = lu_BitstreamRead_u8(state, 8);
   src.beauty = lu_BitstreamRead_u8(state, 8);
   src.cute = lu_BitstreamRead_u8(state, 8);
   src.smart = lu_BitstreamRead_u8(state, 8);
   src.tough = lu_BitstreamRead_u8(state, 8);
   src.active = lu_BitstreamRead_bool(state, src.active);
}
void lu_BitstreamWrite_Roamer(struct lu_BitstreamState* state, const struct Roamer* src) {
   lu_BitstreamWrite_u32(state, src.ivs, 32);
   lu_BitstreamWrite_u32(state, src.personality, 32);
   lu_BitstreamWrite_u16(state, src.species, 11);
   lu_BitstreamWrite_u16(state, src.hp, 16);
   lu_BitstreamWrite_u8(state, src.level, 7);
   lu_BitstreamWrite_u8(state, src.status, 8);
   lu_BitstreamWrite_u8(state, src.cool, 8);
   lu_BitstreamWrite_u8(state, src.beauty, 8);
   lu_BitstreamWrite_u8(state, src.cute, 8);
   lu_BitstreamWrite_u8(state, src.smart, 8);
   lu_BitstreamWrite_u8(state, src.tough, 8);
   lu_BitstreamWrite_bool(state, src.active);
}