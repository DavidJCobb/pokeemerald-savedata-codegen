#include "./tests.serialize//serialize_PyramidBag.h"

#include "global.h" // struct definition

// check constants:
#if FRONTIER_LVL_MODE_COUNT != 2
   #error Constant `FRONTIER_LVL_MODE_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if PYRAMID_BAG_ITEMS_COUNT != 10
   #error Constant `PYRAMID_BAG_ITEMS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if FRONTIER_LVL_MODE_COUNT != 2
   #error Constant `FRONTIER_LVL_MODE_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if PYRAMID_BAG_ITEMS_COUNT != 10
   #error Constant `PYRAMID_BAG_ITEMS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif

// TODO:
// void lu_BitstreamRead_PyramidBag(struct lu_BitstreamState* state, struct PyramidBag* dst);

void lu_BitstreamWrite_PyramidBag(struct lu_BitstreamState* state, const struct PyramidBag* src) {
   {
      u16 i, j;
      for (i = 0; i < FRONTIER_LVL_MODE_COUNT; ++i) { 
         for (j = 0; j < PYRAMID_BAG_ITEMS_COUNT; ++j) { 
               lu_BitstreamWrite_u16(state, src.itemId[i][j], 16);
         }
      }
   }
   {
      u16 i, j;
      for (i = 0; i < FRONTIER_LVL_MODE_COUNT; ++i) { 
         for (j = 0; j < PYRAMID_BAG_ITEMS_COUNT; ++j) { 
               lu_BitstreamWrite_u8(state, src.quantity[i][j], 7);
         }
      }
   }
}