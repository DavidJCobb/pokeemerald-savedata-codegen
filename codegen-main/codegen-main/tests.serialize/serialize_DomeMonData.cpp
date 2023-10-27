#include "./tests.serialize//serialize_DomeMonData.h"

#include "global.h"

// TODO:
// void lu_BitstreamRead_DomeMonData(struct lu_BitstreamState* state, struct DomeMonData* dst);

void lu_BitstreamWrite_DomeMonData(struct lu_BitstreamState* state, const struct DomeMonData* src) {
   {
      u16 i;
      for (i = 0; i < 4; ++i) { 
            lu_BitstreamWrite_u16(state, src.moves[i], 16);
      }
   }
   {
      u16 i;
      for (i = 0; i < 6; ++i) { 
            lu_BitstreamWrite_u8(state, src.evs[i], 8);
      }
   }
   lu_BitstreamWrite_u8(state, src.nature, 8);
}