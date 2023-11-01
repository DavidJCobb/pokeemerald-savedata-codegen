#include "struct-serialize//serialize_Apprentice.h"

#include "global.h" // struct definition

// dependencies
#include "./serialize_ApprenticeMon.h"

#include "string_util.h" // gflib; for StringLength

// check constants:
#if MULTI_PARTY_SIZE != 3
   #error Constant `MULTI_PARTY_SIZE` changed in C, but XML not updated or codegen not re-run!
#endif
#if EASY_CHAT_BATTLE_WORDS_COUNT != 6
   #error Constant `EASY_CHAT_BATTLE_WORDS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if TRAINER_ID_LENGTH != 4
   #error Constant `TRAINER_ID_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif
#if PLAYER_NAME_LENGTH != 7
   #error Constant `PLAYER_NAME_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_Apprentice(struct lu_BitstreamState* state, const struct Apprentice* src) {
   src.id = lu_BitstreamRead_u8(state, 5);
   src.lvlMode = lu_BitstreamRead_u8(state, 2);
   src.numQuestions = lu_BitstreamRead_u8(state, 8);
   src.number = lu_BitstreamRead_u8(state, 8);
   {
      u16 i;
      for (i = 0; i < MULTI_PARTY_SIZE; ++i) { 
            lu_BitstreamRead_ApprenticeMon(state, &src.party[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) { 
            src.speechWon[i] = lu_BitstreamRead_u16(state, 16);
      }
   }
   {
      u16 i;
      for (i = 0; i < TRAINER_ID_LENGTH; ++i) { 
            src.playerId[i] = lu_BitstreamRead_u8(state, 8);
      }
   }
   lu_BitstreamRead_string_optional_terminator(state, src.playerName, PLAYER_NAME_LENGTH, 3);
   src.language = lu_BitstreamRead_u8(state, 3);
   src.checksum = lu_BitstreamRead_u32(state, 32);
}
void lu_BitstreamWrite_Apprentice(struct lu_BitstreamState* state, const struct Apprentice* src) {
   lu_BitstreamWrite_u8(state, src.id, 5);
   lu_BitstreamWrite_u8(state, src.lvlMode, 2);
   lu_BitstreamWrite_u8(state, src.numQuestions, 8);
   lu_BitstreamWrite_u8(state, src.number, 8);
   {
      u16 i;
      for (i = 0; i < MULTI_PARTY_SIZE; ++i) { 
            lu_BitstreamWrite_ApprenticeMon(state, &src.party[i]);
      }
   }
   {
      u16 i;
      for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) { 
            lu_BitstreamWrite_u16(state, src.speechWon[i], 16);
      }
   }
   {
      u16 i;
      for (i = 0; i < TRAINER_ID_LENGTH; ++i) { 
            lu_BitstreamWrite_u8(state, src.playerId[i], 8);
      }
   }
   lu_BitstreamWrite_string_optional_terminator(state, src.playerName, PLAYER_NAME_LENGTH, 3);
   lu_BitstreamWrite_u8(state, src.language, 3);
   lu_BitstreamWrite_u32(state, src.checksum, 32);
}