#include "./tests.serialize//serialize_Apprentice.h"

// dependencies
#include "./serialize_ApprenticeMon.h"

#include "string_util.h" // gflib; for StringLength

#include "global.h"

// TODO:
// void lu_BitstreamRead_Apprentice(struct lu_BitstreamState* state, struct Apprentice* dst);

void lu_BitstreamWrite_Apprentice(struct lu_BitstreamState* state, struct Apprentice* src) {
   lu_BitstreamWrite_u8(state, src.id, 8);
   lu_BitstreamWrite_u8(state, src.lvlMode, 8);
   lu_BitstreamWrite_u8(state, src.numQuestions, 8);
   lu_BitstreamWrite_u8(state, src.number, 8);
   {
      u16 i;
      for (i = 0; i < 3; ++i) { 
            lu_BitstreamWrite_ApprenticeMon(&src.party[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < 6; ++i) { 
            lu_BitstreamWrite_u16(state, src.speechWon[i], 16);
      }
   }
   {
      u16 i;
      for (i = 0; i < 4; ++i) { 
            lu_BitstreamWrite_u8(state, src.playerId[i], 8);
      }
   }
   lu_BitstreamWrite_string(state, src.playerName, 7, 3);
   lu_BitstreamWrite_u8(state, src.language, 3);
   lu_BitstreamWrite_u32(state, src.checksum, 32);
}