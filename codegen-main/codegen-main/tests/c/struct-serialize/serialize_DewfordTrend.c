#include "struct-serialize//serialize_DewfordTrend.h"

#include "global.h" // struct definition

// TODO:
// void lu_BitstreamRead_DewfordTrend(struct lu_BitstreamState* state, struct DewfordTrend* dst);

void lu_BitstreamWrite_DewfordTrend(struct lu_BitstreamState* state, const struct DewfordTrend* src) {
   lu_BitstreamWrite_u8(state, src.trendiness, 7);
   lu_BitstreamWrite_u8(state, src.maxTrendiness, 7);
   lu_BitstreamWrite_bool(state, src.gainingTrendiness);
   lu_BitstreamWrite_u16(state, src.rand, 16);
   {
      u16 i;
      for (i = 0; i < 2; ++i) { 
            lu_BitstreamWrite_u16(state, src.words[i], 16);
      }
   }
}