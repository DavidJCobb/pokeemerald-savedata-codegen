#include "struct-serialize//serialize_Pokedex.h"

#include "global.h" // struct definition

// check constants:
#if NUM_DEX_FLAG_BYTES != 52
   #error Constant `NUM_DEX_FLAG_BYTES` changed in C, but XML not updated or codegen not re-run!
#endif

// TODO:
// void lu_BitstreamRead_Pokedex(struct lu_BitstreamState* state, struct Pokedex* dst);

void lu_BitstreamWrite_Pokedex(struct lu_BitstreamState* state, const struct Pokedex* src) {
   lu_BitstreamWrite_u8(state, src.order, 8);
   lu_BitstreamWrite_u8(state, src.mode, 8);
   lu_BitstreamWrite_u8(state, src.nationalMagic, 8);
   lu_BitstreamWrite_u8(state, src.unknown2, 8);
   lu_BitstreamWrite_u32(state, src.unownPersonality, 32);
   lu_BitstreamWrite_u32(state, src.spindaPersonality, 32);
   lu_BitstreamWrite_u32(state, src.unknown3, 32);
   {
      u16 i;
      for (i = 0; i < NUM_DEX_FLAG_BYTES; ++i) { 
            lu_BitstreamWrite_u8(state, src.owned[i], 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < NUM_DEX_FLAG_BYTES; ++i) { 
            lu_BitstreamWrite_u8(state, src.seen[i], 8);
      }
   }
}