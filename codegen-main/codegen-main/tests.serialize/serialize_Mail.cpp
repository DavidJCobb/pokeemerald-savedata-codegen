#include "./tests.serialize//serialize_Mail.h"

#include "string_util.h" // gflib; for StringLength

#include "global.h"

// TODO:
// void lu_BitstreamRead_Mail(struct lu_BitstreamState* state, struct Mail* dst);

void lu_BitstreamWrite_Mail(struct lu_BitstreamState* state, const struct Mail* src) {
   {
      u16 i;
      for (i = 0; i < 9; ++i) { 
            lu_BitstreamWrite_u16(state, src.words[i], 16);
      }
   }
   lu_BitstreamWrite_string(state, src.playerName, 7, 3);
   {
      u16 i;
      for (i = 0; i < 4; ++i) { 
            lu_BitstreamWrite_u8(state, src.trainerId[i], 8);
      }
   }
   lu_BitstreamWrite_u16(state, src.species, 11);
   lu_BitstreamWrite_u16(state, src.itemId, 16);
}