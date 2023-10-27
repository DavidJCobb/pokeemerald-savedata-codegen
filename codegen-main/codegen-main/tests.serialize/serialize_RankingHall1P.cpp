#include "./tests.serialize//serialize_RankingHall1P.h"

#include "global.h" // struct definition

#include "string_util.h" // gflib; for StringLength

// check constants:
#if TRAINER_ID_LENGTH != 4
   #error Constant `TRAINER_ID_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif
#if PLAYER_NAME_LENGTH != 7
   #error Constant `PLAYER_NAME_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif

// TODO:
// void lu_BitstreamRead_RankingHall1P(struct lu_BitstreamState* state, struct RankingHall1P* dst);

void lu_BitstreamWrite_RankingHall1P(struct lu_BitstreamState* state, const struct RankingHall1P* src) {
   {
      u16 i;
      for (i = 0; i < TRAINER_ID_LENGTH; ++i) { 
            lu_BitstreamWrite_u8(state, src.id[i], 8);
      }
   }
   lu_BitstreamWrite_u16(state, src.winStreak, 16);
   lu_BitstreamWrite_string(state, src.name, PLAYER_NAME_LENGTH, 3);
   lu_BitstreamWrite_u8(state, src.language, 3);
}