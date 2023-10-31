#include "struct-serialize//serialize_Roamer.h"

#include "global.h" // struct definition

// TODO:
// void lu_BitstreamRead_Roamer(struct lu_BitstreamState* state, struct Roamer* dst);

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
   {
      u16 i;
      for (i = 0; i < 8; ++i) { 
            lu_BitstreamWrite_u8(state, src.filler[i], 8);
      }
   }
}