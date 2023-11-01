#include "lu/generated/struct-serialize//serialize_BerryCrush.h"

#include "global.h" // struct definition

void lu_BitstreamRead_BerryCrush(struct lu_BitstreamState* state, const struct BerryCrush* src) {
   {
      u16 i;
      for (i = 0; i < 4; ++i) { 
            src.pressingSpeeds[i] = lu_BitstreamRead_u16(state, 16);
      }
   }
   src.berryPowderAmount = lu_BitstreamRead_u32(state, 32);
   src.unk = lu_BitstreamRead_u32(state, 32);
}
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