#include "./tests.serialize//serialize_BattleTowerEReaderTrainer.h"

// dependencies
#include "./serialize_BattleTowerPokemon.h"

#include "string_util.h" // gflib; for StringLength

#include "global.h"

// TODO:
// void lu_BitstreamRead_BattleTowerEReaderTrainer(struct lu_BitstreamState* state, struct BattleTowerEReaderTrainer* dst);

void lu_BitstreamWrite_BattleTowerEReaderTrainer(struct lu_BitstreamState* state, struct BattleTowerEReaderTrainer* src) {
   lu_BitstreamWrite_u8(state, src.unk0, 8);
   lu_BitstreamWrite_u8(state, src.facilityClass, 8);
   lu_BitstreamWrite_u16(state, src.winStreak, 16);
   lu_BitstreamWrite_string(state, src.name, 7, 3);
   {
      u16 i;
      for (i = 0; i < 4; ++i) { 
            lu_BitstreamWrite_u8(state, src.trainerId[i], 8);
      }
   }
   {
      u16 i;
      for (i = 0; i < 6; ++i) { 
            lu_BitstreamWrite_u16(state, src.greeting[i], 16);
      }
   }
   {
      u16 i;
      for (i = 0; i < 6; ++i) { 
            lu_BitstreamWrite_u16(state, src.farewellPlayerLost[i], 16);
      }
   }
   {
      u16 i;
      for (i = 0; i < 6; ++i) { 
            lu_BitstreamWrite_u16(state, src.farewellPlayerWon[i], 16);
      }
   }
   {
      u16 i;
      for (i = 0; i < 3; ++i) { 
            lu_BitstreamWrite_BattleTowerPokemon(&src.party[i]);
      }
   }
   lu_BitstreamWrite_u32(state, src.checksum, 32);
}