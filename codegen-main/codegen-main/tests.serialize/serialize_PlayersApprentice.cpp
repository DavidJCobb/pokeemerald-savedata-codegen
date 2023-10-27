#include "./tests.serialize//serialize_PlayersApprentice.h"

// dependencies
#include "./serialize_ApprenticeQuestion.h"

#include "global.h"

// TODO:
// void lu_BitstreamRead_PlayersApprentice(struct lu_BitstreamState* state, struct PlayersApprentice* dst);

void lu_BitstreamWrite_PlayersApprentice(struct lu_BitstreamState* state, const struct PlayersApprentice* src) {
   lu_BitstreamWrite_u8(state, src.id, 8);
   lu_BitstreamWrite_u8(state, src.lvlMode, 2);
   lu_BitstreamWrite_u8(state, src.questionsAnswered, 4);
   lu_BitstreamWrite_u8(state, src.leadMonId, 2);
   lu_BitstreamWrite_u8(state, src.party, 3);
   lu_BitstreamWrite_u8(state, src.saveId, 2);
   lu_BitstreamWrite_u8(state, src.unused, 8);
   {
      u16 i;
      for (i = 0; i < 3; ++i) { 
            lu_BitstreamWrite_u8(state, src.speciesIds[i], 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < 9; ++i) { 
            lu_BitstreamWrite_ApprenticeQuestion(state, &src.questions[i]);
      }
   }
}