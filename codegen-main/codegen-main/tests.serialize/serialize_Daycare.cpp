#include "./tests.serialize//serialize_Daycare.h"

// dependencies
#include "./serialize_DaycareMon.h"

#include "global.h"

// TODO:
// void lu_BitstreamRead_Daycare(struct lu_BitstreamState* state, struct Daycare* dst);

void lu_BitstreamWrite_Daycare(struct lu_BitstreamState* state, struct Daycare* src) {
   {
      u16 i;
      for (i = 0; i < 2; ++i) { 
            lu_BitstreamWrite_DaycareMon(&src.mons[i]);
      }
   }
   lu_BitstreamWrite_u32(state, src.offspringPersonality, 32);
   lu_BitstreamWrite_u8(state, src.stepCounter, 8);
}