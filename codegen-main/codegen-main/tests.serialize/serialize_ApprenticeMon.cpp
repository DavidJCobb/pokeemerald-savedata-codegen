#include "./tests.serialize//serialize_ApprenticeMon.h"

#include "global.h"

// TODO:
// void lu_BitstreamRead_ApprenticeMon(struct lu_BitstreamState* state, struct ApprenticeMon* dst);

void lu_BitstreamWrite_ApprenticeMon(struct lu_BitstreamState* state, const struct ApprenticeMon* src) {
   lu_BitstreamWrite_u16(state, src.species, 11);
   {
      u16 i;
      for (i = 0; i < 4; ++i) { 
            lu_BitstreamWrite_u16(state, src.moves[i], 16);
      }
   }
   lu_BitstreamWrite_u16(state, src.item, 16);
}