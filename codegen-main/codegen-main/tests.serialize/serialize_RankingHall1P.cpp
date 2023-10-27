#include "./tests.serialize//serialize_RankingHall1P.h"

#include "string_util.h" // gflib; for StringLength

#include "global.h"

// TODO:
// void lu_BitstreamRead_RankingHall1P(struct lu_BitstreamState* state, struct RankingHall1P* dst);

void lu_BitstreamWrite_RankingHall1P(struct lu_BitstreamState* state, struct RankingHall1P* src) {
   {
      u16 i;
      for (i = 0; i < 4; ++i) { 
            lu_BitstreamWrite_u8(state, src.id[i], 8);
      }
   }
   lu_BitstreamWrite_u16(state, src.winStreak, 16);
   lu_BitstreamWrite_string(state, src.name, 7, 3);
   lu_BitstreamWrite_u8(state, src.language, 3);
}