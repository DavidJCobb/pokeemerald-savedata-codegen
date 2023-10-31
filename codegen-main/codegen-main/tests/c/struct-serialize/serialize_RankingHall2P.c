#include "struct-serialize//serialize_RankingHall2P.h"

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
// void lu_BitstreamRead_RankingHall2P(struct lu_BitstreamState* state, struct RankingHall2P* dst);

void lu_BitstreamWrite_RankingHall2P(struct lu_BitstreamState* state, const struct RankingHall2P* src) {
   {
      u16 i;
      for (i = 0; i < TRAINER_ID_LENGTH; ++i) { 
            lu_BitstreamWrite_u8(state, src.id1[i], 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < TRAINER_ID_LENGTH; ++i) { 
            lu_BitstreamWrite_u8(state, src.id2[i], 8);
      }
   }
   lu_BitstreamWrite_u16(state, src.winStreak, 16);
   lu_BitstreamWrite_string(state, src.name1, PLAYER_NAME_LENGTH, 3);
   lu_BitstreamWrite_string(state, src.name2, PLAYER_NAME_LENGTH, 3);
   lu_BitstreamWrite_u8(state, src.language, 3);
}