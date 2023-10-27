#include "./tests.serialize//serialize_Mail.h"

#include "global.h" // struct definition

#include "string_util.h" // gflib; for StringLength

// check constants:
#if MAIL_WORDS_COUNT != 9
   #error Constant `MAIL_WORDS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if PLAYER_NAME_LENGTH != 7
   #error Constant `PLAYER_NAME_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif
#if TRAINER_ID_LENGTH != 4
   #error Constant `TRAINER_ID_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif

// TODO:
// void lu_BitstreamRead_Mail(struct lu_BitstreamState* state, struct Mail* dst);

void lu_BitstreamWrite_Mail(struct lu_BitstreamState* state, const struct Mail* src) {
   {
      u16 i;
      for (i = 0; i < MAIL_WORDS_COUNT; ++i) { 
            lu_BitstreamWrite_u16(state, src.words[i], 16);
      }
   }
   lu_BitstreamWrite_string(state, src.playerName, PLAYER_NAME_LENGTH, 3);
   {
      u16 i;
      for (i = 0; i < TRAINER_ID_LENGTH; ++i) { 
            lu_BitstreamWrite_u8(state, src.trainerId[i], 8);
      }
   }
   lu_BitstreamWrite_u16(state, src.species, 11);
   lu_BitstreamWrite_u16(state, src.itemId, 16);
}