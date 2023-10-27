#include "./tests.serialize//serialize_RankingHall2P.h"

#include "string_util.h" // gflib; for StringLength

#include "global.h"

// TODO:
// void lu_BitstreamRead_RankingHall2P(struct lu_BitstreamState* state, struct RankingHall2P* dst);

void lu_BitstreamWrite_RankingHall2P(struct lu_BitstreamState* state, const struct RankingHall2P* src) {
   {
      u16 i;
      for (i = 0; i < 4; ++i) { 
            lu_BitstreamWrite_u8(state, src.id1[i], 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < 4; ++i) { 
            lu_BitstreamWrite_u8(state, src.id2[i], 8);
      }
   }
   lu_BitstreamWrite_u16(state, src.winStreak, 16);
   lu_BitstreamWrite_string(state, src.name1, 7, 3);
   lu_BitstreamWrite_string(state, src.name2, 7, 3);
   lu_BitstreamWrite_u8(state, src.language, 3);
}