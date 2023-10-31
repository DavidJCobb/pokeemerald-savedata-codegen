#include "struct-serialize//serialize_EmeraldBattleTowerRecord.h"

#include "global.h" // struct definition

// dependencies
#include "./serialize_BattleTowerPokemon.h"

#include "string_util.h" // gflib; for StringLength

// check constants:
#if PLAYER_NAME_LENGTH != 7
   #error Constant `PLAYER_NAME_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif
#if TRAINER_ID_LENGTH != 4
   #error Constant `TRAINER_ID_LENGTH` changed in C, but XML not updated or codegen not re-run!
#endif
#if EASY_CHAT_BATTLE_WORDS_COUNT != 6
   #error Constant `EASY_CHAT_BATTLE_WORDS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if FRONTIER_PARTY_SIZE != 3
   #error Constant `FRONTIER_PARTY_SIZE` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_EmeraldBattleTowerRecord(struct lu_BitstreamState* state, const struct EmeraldBattleTowerRecord* src) {
   src.lvlMode = lu_BitstreamRead_u8(state, 8);
   src.facilityClass = lu_BitstreamRead_u8(state, 8);
   src.winStreak = lu_BitstreamRead_u16(state, 16);
   lu_BitstreamRead_string(state, src.name, PLAYER_NAME_LENGTH, 3);
   {
      u16 i;
      for (i = 0; i < TRAINER_ID_LENGTH; ++i) { 
            src.trainerId[i] = lu_BitstreamRead_u8(state, 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) { 
            src.greeting[i] = lu_BitstreamRead_u16(state, 16);
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
      for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) { 
            src.speechLost[i] = lu_BitstreamRead_u16(state, 16);
      }
   }
   {
      u16 i;
      for (i = 0; i < FRONTIER_PARTY_SIZE; ++i) { 
            lu_BitstreamRead_BattleTowerPokemon(state, &src.party[i]);
      }
   }
   src.language = lu_BitstreamRead_u8(state, 3);
   src.checksum = lu_BitstreamRead_u32(state, 32);
}
void lu_BitstreamWrite_EmeraldBattleTowerRecord(struct lu_BitstreamState* state, const struct EmeraldBattleTowerRecord* src) {
   lu_BitstreamWrite_u8(state, src.lvlMode, 8);
   lu_BitstreamWrite_u8(state, src.facilityClass, 8);
   lu_BitstreamWrite_u16(state, src.winStreak, 16);
   lu_BitstreamWrite_string(state, src.name, PLAYER_NAME_LENGTH, 3);
   {
      u16 i;
      for (i = 0; i < TRAINER_ID_LENGTH; ++i) { 
            lu_BitstreamWrite_u8(state, src.trainerId[i], 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) { 
            lu_BitstreamWrite_u16(state, src.greeting[i], 16);
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
      for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) { 
            lu_BitstreamWrite_u16(state, src.speechLost[i], 16);
      }
   }
   {
      u16 i;
      for (i = 0; i < FRONTIER_PARTY_SIZE; ++i) { 
            lu_BitstreamWrite_BattleTowerPokemon(state, &src.party[i]);
      }
   }
   lu_BitstreamWrite_u8(state, src.language, 3);
   lu_BitstreamWrite_u32(state, src.checksum, 32);
}