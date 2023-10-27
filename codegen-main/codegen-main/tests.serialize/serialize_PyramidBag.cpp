#include "./tests.serialize//serialize_PyramidBag.h"

#include "global.h"

// TODO:
// void lu_BitstreamRead_PyramidBag(struct lu_BitstreamState* state, struct PyramidBag* dst);

void lu_BitstreamWrite_PyramidBag(struct lu_BitstreamState* state, const struct PyramidBag* src) {
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < 10; ++j) { 
               lu_BitstreamWrite_u16(state, src.itemId[i][j], 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < 2; ++i) { 
         for (j = 0; j < 10; ++j) { 
               lu_BitstreamWrite_u8(state, src.quantity[i][j], 7);
         }
      }
   }
}