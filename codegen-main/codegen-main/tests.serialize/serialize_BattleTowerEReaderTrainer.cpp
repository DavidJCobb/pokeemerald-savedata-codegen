#include "./tests.serialize//serialize_BattleTowerEReaderTrainer.h"

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

// TODO:
// void lu_BitstreamRead_BattleTowerEReaderTrainer(struct lu_BitstreamState* state, struct BattleTowerEReaderTrainer* dst);

void lu_BitstreamWrite_BattleTowerEReaderTrainer(struct lu_BitstreamState* state, const struct BattleTowerEReaderTrainer* src) {
   lu_BitstreamWrite_u8(state, src.unk0, 8);
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
            lu_BitstreamWrite_u16(state, src.farewellPlayerLost[i], 16);
      }
   }
   {
      u16 i;
      for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; ++i) { 
            lu_BitstreamWrite_u16(state, src.farewellPlayerWon[i], 16);
      }
   }
   {
      u16 i;
      for (i = 0; i < FRONTIER_PARTY_SIZE; ++i) { 
            lu_BitstreamWrite_BattleTowerPokemon(state, &src.party[i]);
      }
   }
   lu_BitstreamWrite_u32(state, src.checksum, 32);
}