#include "./tests.serialize//serialize_BerryCrush.h"

#include "global.h" // struct definition

// TODO:
// void lu_BitstreamRead_BerryCrush(struct lu_BitstreamState* state, struct BerryCrush* dst);

void lu_BitstreamWrite_BerryCrush(struct lu_BitstreamState* state, const struct BerryCrush* src) {
   {
      u16 i;
      for (i = 0; i < 4; ++i) { 
            lu_BitstreamWrite_u16(state, src.pressingSpeeds[i], 16);
      }
   }
   lu_BitstreamWrite_u32(state, src.berryPowderAmount, 32);
   lu_BitstreamWrite_u32(state, src.unk, 32);
}