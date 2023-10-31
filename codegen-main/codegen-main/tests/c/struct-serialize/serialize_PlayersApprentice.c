#include "struct-serialize//serialize_PlayersApprentice.h"

#include "global.h" // struct definition

// dependencies
#include "./serialize_ApprenticeQuestion.h"

// check constants:
#if MULTI_PARTY_SIZE != 3
   #error Constant `MULTI_PARTY_SIZE` changed in C, but XML not updated or codegen not re-run!
#endif
#if APPRENTICE_MAX_QUESTIONS != 9
   #error Constant `APPRENTICE_MAX_QUESTIONS` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_PlayersApprentice(struct lu_BitstreamState* state, const struct PlayersApprentice* src) {
   src.id = lu_BitstreamRead_u8(state, 8);
   src.lvlMode = lu_BitstreamRead_u8(state, 2);
   src.questionsAnswered = lu_BitstreamRead_u8(state, 4);
   src.leadMonId = lu_BitstreamRead_u8(state, 2);
   src.party = lu_BitstreamRead_u8(state, 3);
   src.saveId = lu_BitstreamRead_u8(state, 2);
   src.unused = lu_BitstreamRead_u8(state, 8);
   {
      u16 i;
      for (i = 0; i < MULTI_PARTY_SIZE; ++i) { 
            src.speciesIds[i] = lu_BitstreamRead_u8(state, 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < APPRENTICE_MAX_QUESTIONS; ++i) { 
            lu_BitstreamRead_ApprenticeQuestion(state, &src.questions[i]);
      }
   }
}
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
      for (i = 0; i < MULTI_PARTY_SIZE; ++i) { 
            lu_BitstreamWrite_u8(state, src.speciesIds[i], 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < APPRENTICE_MAX_QUESTIONS; ++i) { 
            lu_BitstreamWrite_ApprenticeQuestion(state, &src.questions[i]);
      }
   }
}