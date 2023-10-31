#include "struct-serialize//serialize_EnigmaBerry.h"

#include "global.berry.h" // struct definition

// dependencies
#include "./serialize_Berry2.h"

// check constants:
#if BERRY_ITEM_EFFECT_COUNT != 18
   #error Constant `BERRY_ITEM_EFFECT_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif

// TODO:
// void lu_BitstreamRead_EnigmaBerry(struct lu_BitstreamState* state, struct EnigmaBerry* dst);

void lu_BitstreamWrite_EnigmaBerry(struct lu_BitstreamState* state, const struct EnigmaBerry* src) {
   lu_BitstreamWrite_Berry2(state, &src.berry);
   {
      u16 i;
      for (i = 0; i < BERRY_ITEM_EFFECT_COUNT; ++i) { 
            lu_BitstreamWrite_u8(state, src.itemEffect[i], 8);
      }
   }
   lu_BitstreamWrite_u8(state, src.holdEffect, 8);
   lu_BitstreamWrite_u8(state, src.holdEffectParam, 8);
   lu_BitstreamWrite_u32(state, src.checksum, 32);
}